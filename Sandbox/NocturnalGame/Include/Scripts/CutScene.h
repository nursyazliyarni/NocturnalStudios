/*!------------------------------------------------------------------------------------------------
    File: CutScene.h
    Project: NocturnalStudio

	Brief:
		 This file contains the CutScene class declaration
		 which is responsible for managing cutscene functionalities within the engine 
		 for the cutscene components.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the CutScene.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Scripts/IScript.h"
#include "Input/Input.h"
#include "Graphics/Camera2D.h"
#include "Graphics/Graphics.h"
#include "ECS/SceneManager.h"
#include "GUI/GUI.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Cutscenes Script
	****************************************************************************/
	class CutScene : public IScript
	{
	public:
		/*!*************************************************************************
		Default constructor for Cutscenes
		****************************************************************************/
		// Cache input system so we can detect mouse clicks.
		CutScene() : mInput{ InputSystem::GetInstance() } {}
		/*!*************************************************************************
		Default destructor for Cutscenes
		****************************************************************************/
		~CutScene() = default;
		/*!*************************************************************************
		Returns a new copy of Cutscenes Script
		****************************************************************************/
		virtual CutScene* Clone() const override
		{
			return new CutScene(*this);
		}
		/*!*************************************************************************
		Start State of Cutscenes Script
		****************************************************************************/
		virtual void Start() override
		{
			// Reset runtime state whenever this scene starts.
			mCutImages.clear();
			mOriginalScales.clear();
			mCurrentImage = 0;
			mNextButton = 0;

			// Keep camera centered for cutscene presentation.
			NocturnalEngine::Graphic::mcamera->SetPosition({ 0.0f, 0.0f, 0.0f });

			// Find all cutscene image entities named:
			// CutImage1, CutImage2, ... CutImageN.
			std::vector<std::pair<int, Entity>> indexedImages{};
			for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i)
			{
				if (!p_ecs.HaveComponent<NameTag>(i))
					continue;

				const std::string& name = p_ecs.GetComponent<NameTag>(i).GetNameTag();
				// Save Next button entity for click checks.
				if (name == "NextButton")
				{
					mNextButton = i;
					continue;
				}

				// Ignore entities that are not cutscene images.
				if (name.rfind("CutImage", 0) != 0)
					continue;

				// Cut images must have transform + sprite.
				if (!p_ecs.HaveComponent<Transform>(i) || !p_ecs.HaveComponent<Sprite>(i))
					continue;

				// Read number from "CutImageX" so we can sort in correct order.
				std::string suffix = name.substr(std::string("CutImage").size());
				if (suffix.empty() || !std::all_of(suffix.begin(), suffix.end(),
					[](unsigned char ch) { return static_cast<bool>(std::isdigit(ch)); }))
				{
					continue;
				}

				indexedImages.push_back({ std::stoi(suffix), i });
			}

			std::sort(indexedImages.begin(), indexedImages.end(),
				[](const std::pair<int, Entity>& a, const std::pair<int, Entity>& b) {
					return a.first < b.first;
				});

			// Store ordered entities and their original scales for show/hide toggling.
			for (const auto& pair : indexedImages)
			{
				Entity e = pair.second;
				mCutImages.push_back(e);
				mOriginalScales.push_back(p_ecs.GetComponent<Transform>(e).GetScale());
			}

			// Show first image, hide all others.
			ApplyImageVisibility();
		}
		/*!*************************************************************************
		Update Loop of Cutscenes Script
		****************************************************************************/
		virtual void Update(float Frametime) override
		{
			UNREFERENCED_PARAMETER(Frametime);
			if (mCutImages.empty())
				return;

			// Advance only when player clicks inside Next button collider.
			if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && IsMouseOnNextButton())
			{
				// Stop at last image; do not overflow index.
				if (mCurrentImage + 1 < mCutImages.size())
				{
					++mCurrentImage;
					// Update which image is visible.
					ApplyImageVisibility();
				}
			}
		}	
		/*!*************************************************************************
		End State for Cutscenes
		****************************************************************************/
		virtual void End() override
		{
			
		}

		bool Requirements(Entity e) override {
			// Script can be attached broadly; runtime checks handle exact entities.
			UNREFERENCED_PARAMETER(e);
			return true;
		}
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "CutScene";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		bool IsMouseOnNextButton()
		{
			// Need both collider and transform for hit testing.
			if (mNextButton == 0 ||
				!p_ecs.HaveComponent<Collider>(mNextButton) ||
				!p_ecs.HaveComponent<Transform>(mNextButton))
			{
				return false;
			}

			// Mouse position in world/UI space used by this project.
			const vec2D mPos{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
			Collider& col = p_ecs.GetComponent<Collider>(mNextButton);
			Transform& t = p_ecs.GetComponent<Transform>(mNextButton);

			// Check both collider slots used by this engine.
			for (int i = 0; i < 2; ++i)
			{
				const vec2D offsetPos{ t.GetPos().x + col[i].mOffset.x, t.GetPos().y + col[i].mOffset.y };
				switch (col[i].mCol)
				{
				case Collider::ColliderType::circle:
				case Collider::ColliderType::bubble:
				case Collider::ColliderType::bossball:
					// Circle hit test.
					if (distance(mPos, offsetPos) < col[i].mRadius) return true;
					break;

				case Collider::ColliderType::rect:
				case Collider::ColliderType::box:
				{
					// Box hit test.
					const vec2D minPt{ offsetPos.x - col[i].mMin.x, offsetPos.y - col[i].mMin.y };
					const vec2D maxPt{ offsetPos.x + col[i].mMax.x, offsetPos.y + col[i].mMax.y };
					if (mPos.x > minPt.x && mPos.x < maxPt.x &&
						mPos.y > minPt.y && mPos.y < maxPt.y)
					{
						return true;
					}
					break;
				}

				default:
					break;
				}
			}
			return false;
		}

		void ApplyImageVisibility()
		{
			// Show only current image by restoring scale.
			// Hide others by setting scale to zero.
			for (size_t i = 0; i < mCutImages.size(); ++i)
			{
				Entity e = mCutImages[i];
				if (!p_ecs.HaveComponent<Transform>(e))
					continue;

				Transform& t = p_ecs.GetComponent<Transform>(e);
				if (i == mCurrentImage)
				{
					t.SetScale(mOriginalScales[i].x, mOriginalScales[i].y);
				}
				else
				{
					t.SetScale(0.0f, 0.0f);
				}
			}
		}

		std::unique_ptr<InputSystem>& mInput;
		std::vector<Entity> mCutImages{};
		std::vector<vec2D> mOriginalScales{};
		size_t mCurrentImage{ 0 };
		Entity mNextButton{ 0 };
	};
}
