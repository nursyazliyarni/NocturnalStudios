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
#include "ECS/Components/Text.h"
#include "ECS/Components/Video.h"
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
				mCutTextGroups.clear();
				mOriginalTextAlpha.clear();
				mOriginalTextContent.clear();
				// Preserve each layer's original video settings before cutscene toggling overrides them.
				mOriginalVideoEnabled.clear();
				mOriginalVideoSkipOnMouseClick.clear();
				mTypewriterElapsed = 0.0f;
				mCurrentImage = 0;
				mCurrentTextLine = 0;
				mNextButton = 0;

			// Keep camera centered for cutscene presentation.
			NocturnalEngine::Graphic::mcamera->SetPosition({ 0.0f, 0.0f, 0.0f });

				// Find all cutscene image entities named:
				// CutImage1, CutImage2, ... CutImageN.
				std::vector<std::pair<int, Entity>> indexedImages{};
				std::vector<std::pair<int, Entity>> indexedOverlays{};
				std::vector<std::tuple<int, int, Entity>> indexedTexts{};
				Entity cutVideoEntity = MAX_ENTITIES;
				int maxCutImageIndex = 0;
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

					if (name.rfind("CutText", 0) == 0 && p_ecs.HaveComponent<Text>(i))
					{
						const std::string suffix = name.substr(std::string("CutText").size());
						const std::size_t separator = suffix.find('_');
						if (separator != std::string::npos)
						{
							const std::string imagePart = suffix.substr(0, separator);
							const std::string linePart = suffix.substr(separator + 1);
							if (!imagePart.empty() && !linePart.empty() &&
								std::all_of(imagePart.begin(), imagePart.end(), [](unsigned char ch) { return static_cast<bool>(std::isdigit(ch)); }) &&
								std::all_of(linePart.begin(), linePart.end(), [](unsigned char ch) { return static_cast<bool>(std::isdigit(ch)); }))
							{
								indexedTexts.emplace_back(std::stoi(imagePart), std::stoi(linePart), i);
							}
						}
						continue;
					}

					if (name.rfind("CutOverlay", 0) == 0)
					{
						const std::string suffix = name.substr(std::string("CutOverlay").size());
						std::size_t digitCount = 0;
						while (digitCount < suffix.size() && std::isdigit(static_cast<unsigned char>(suffix[digitCount])))
						{
							++digitCount;
						}

						if (digitCount == 0)
							continue;

						if (!p_ecs.HaveComponent<Transform>(i) || !p_ecs.HaveComponent<Sprite>(i))
							continue;

						indexedOverlays.push_back({ std::stoi(suffix.substr(0, digitCount)), i });
						continue;
					}

					// Ignore entities that are not cutscene images.
					if (name == "CutVideo")
					{
						cutVideoEntity = i;
						continue;
					}

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

				const int cutImageIndex = std::stoi(suffix);
				indexedImages.push_back({ cutImageIndex, i });
				maxCutImageIndex = std::max(maxCutImageIndex, cutImageIndex);
			}

				if (cutVideoEntity != MAX_ENTITIES &&
					p_ecs.HaveComponent<Transform>(cutVideoEntity) &&
					p_ecs.HaveComponent<Sprite>(cutVideoEntity))
				{
					// Treat the video as the final cutscene layer even though it has its own explicit name.
					indexedImages.push_back({ maxCutImageIndex + 1, cutVideoEntity });
				}

				std::sort(indexedImages.begin(), indexedImages.end(),
					[](const std::pair<int, Entity>& a, const std::pair<int, Entity>& b) {
						return a.first < b.first;
					});

				std::sort(indexedTexts.begin(), indexedTexts.end(),
					[](const std::tuple<int, int, Entity>& a, const std::tuple<int, int, Entity>& b) {
						if (std::get<0>(a) != std::get<0>(b))
							return std::get<0>(a) < std::get<0>(b);
						return std::get<1>(a) < std::get<1>(b);
					});

				// Store ordered entities and their original scales for show/hide toggling.
				for (const auto& pair : indexedImages)
				{
					Entity e = pair.second;
					mCutImages.push_back(e);
					mOriginalScales.push_back(p_ecs.GetComponent<Transform>(e).GetScale());
					// Cache per-layer video defaults so they can be restored when the layer becomes active again.
					mOriginalVideoEnabled.push_back(p_ecs.HaveComponent<Video>(e)
						? p_ecs.GetComponent<Video>(e).enabled
						: false);
					mOriginalVideoSkipOnMouseClick.push_back(p_ecs.HaveComponent<Video>(e)
						? p_ecs.GetComponent<Video>(e).skipOnMouseClick
						: false);
				}

				mCutOverlayGroups.resize(mCutImages.size());
				mOriginalOverlayScales.resize(mCutImages.size());
				mOriginalOverlayVideoEnabled.resize(mCutImages.size());
				mOriginalOverlayVideoSkipOnMouseClick.resize(mCutImages.size());
				for (const auto& [imageNumber, overlayEntity] : indexedOverlays)
				{
					const int imageIndex = imageNumber - 1;
					if (imageIndex < 0 || imageIndex >= static_cast<int>(mCutOverlayGroups.size()))
						continue;

					mCutOverlayGroups[imageIndex].push_back(overlayEntity);
					mOriginalOverlayScales[imageIndex].push_back(
						p_ecs.GetComponent<Transform>(overlayEntity).GetScale());
					mOriginalOverlayVideoEnabled[imageIndex].push_back(
						p_ecs.HaveComponent<Video>(overlayEntity)
						? p_ecs.GetComponent<Video>(overlayEntity).enabled
						: false);
					mOriginalOverlayVideoSkipOnMouseClick[imageIndex].push_back(
						p_ecs.HaveComponent<Video>(overlayEntity)
						? p_ecs.GetComponent<Video>(overlayEntity).skipOnMouseClick
						: false);
				}

				mCutTextGroups.resize(mCutImages.size());
				mOriginalTextAlpha.resize(mCutImages.size());
				mOriginalTextContent.resize(mCutImages.size());
				for (const auto& entry : indexedTexts)
				{
					const int imageIndex = std::get<0>(entry) - 1;
					const Entity textEntity = std::get<2>(entry);
					if (imageIndex < 0 || imageIndex >= static_cast<int>(mCutTextGroups.size()))
						continue;

					auto& text = p_ecs.GetComponent<Text>(textEntity);
					mCutTextGroups[imageIndex].push_back(textEntity);
					mOriginalTextAlpha[imageIndex].push_back(text.GetColor()[3]);
					mOriginalTextContent[imageIndex].push_back(text.GetContent());
				}

				// Show first image, hide all others.
				ApplyImageVisibility();
				ApplyTextVisibility();
			}
		/*!*************************************************************************
		Update Loop of Cutscenes Script
		****************************************************************************/
			virtual void Update(float Frametime) override
			{
				if (mCutImages.empty())
					return;

				UpdateTypewriter(Frametime);
				if (TryAdvanceOverlayVideo())
				{
					ApplyImageVisibility();
					ApplyTextVisibility();
					return;
				}
		
					// Advance on any left click.
					if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
					{
						if (!IsCurrentLineFullyRevealed())
						{
							RevealCurrentLineFully();
							return;
						}

						if (HasPendingOverlayVideo())
							return;

						if (AdvanceCutScene())
						{
							ApplyImageVisibility();
							ApplyTextVisibility();
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

				if (p_ecs.HaveComponent<Video>(e))
				{
					auto& video = p_ecs.GetComponent<Video>(e);
					// Hidden cutscene layers must not advance video playback in the background.
					video.enabled = (i == mCurrentImage) ? mOriginalVideoEnabled[i] : false;
					// The active cutscene video should always be skippable by clicking anywhere.
						video.skipOnMouseClick = (i == mCurrentImage) ? true : mOriginalVideoSkipOnMouseClick[i];
				}

				if (i < mCutOverlayGroups.size())
				{
					for (size_t overlayIndex = 0; overlayIndex < mCutOverlayGroups[i].size(); ++overlayIndex)
					{
						Entity overlayEntity = mCutOverlayGroups[i][overlayIndex];
						if (!p_ecs.HaveComponent<Transform>(overlayEntity))
							continue;

						Transform& overlayTransform = p_ecs.GetComponent<Transform>(overlayEntity);
						if (i == mCurrentImage)
						{
							overlayTransform.SetScale(
								mOriginalOverlayScales[i][overlayIndex].x,
								mOriginalOverlayScales[i][overlayIndex].y);
						}
						else
						{
							overlayTransform.SetScale(0.0f, 0.0f);
						}

						if (p_ecs.HaveComponent<Video>(overlayEntity))
						{
							auto& overlayVideo = p_ecs.GetComponent<Video>(overlayEntity);
							overlayVideo.enabled = (i == mCurrentImage)
								? mOriginalOverlayVideoEnabled[i][overlayIndex]
								: false;
							overlayVideo.skipOnMouseClick = (i == mCurrentImage)
								? mOriginalOverlayVideoSkipOnMouseClick[i][overlayIndex]
								: false;
						}
					}
				}
				}
			}

			void ApplyTextVisibility()
			{
				for (size_t imageIndex = 0; imageIndex < mCutTextGroups.size(); ++imageIndex)
				{
					for (size_t lineIndex = 0; lineIndex < mCutTextGroups[imageIndex].size(); ++lineIndex)
					{
						Entity textEntity = mCutTextGroups[imageIndex][lineIndex];
						if (!p_ecs.HaveComponent<Text>(textEntity))
							continue;

						auto& text = p_ecs.GetComponent<Text>(textEntity);
						float* color = text.GetColor();
						color[3] = (imageIndex == mCurrentImage && lineIndex == mCurrentTextLine)
							? mOriginalTextAlpha[imageIndex][lineIndex]
							: 0.0f;

						// Reset the visible line to an empty string so the typewriter effect can replay on entry.
						text.SetContent((imageIndex == mCurrentImage && lineIndex == mCurrentTextLine)
							? std::string{}
							: mOriginalTextContent[imageIndex][lineIndex]);
					}
				}

				mTypewriterElapsed = 0.0f;
			}

			bool AdvanceCutScene()
			{
				if (mCurrentImage >= mCutImages.size())
					return false;

				const auto& currentTextGroup = mCutTextGroups[mCurrentImage];
				if (!currentTextGroup.empty() && mCurrentTextLine + 1 < currentTextGroup.size())
				{
					++mCurrentTextLine;
					return true;
				}

				if (mCurrentImage + 1 < mCutImages.size())
				{
					++mCurrentImage;
					mCurrentTextLine = 0;
					return true;
				}

				return false;
			}

			void UpdateTypewriter(float frametime)
			{
				if (mCurrentImage >= mCutTextGroups.size() ||
					mCurrentTextLine >= mCutTextGroups[mCurrentImage].size())
					return;

				Entity textEntity = mCutTextGroups[mCurrentImage][mCurrentTextLine];
				if (!p_ecs.HaveComponent<Text>(textEntity))
					return;

				const std::string& fullText = mOriginalTextContent[mCurrentImage][mCurrentTextLine];
				if (fullText.empty())
					return;

				mTypewriterElapsed += frametime;
				const std::size_t visibleCharacters = std::min(
					fullText.size(),
					static_cast<std::size_t>(mTypewriterElapsed * mTypewriterCharactersPerSecond));
				p_ecs.GetComponent<Text>(textEntity).SetContent(fullText.substr(0, visibleCharacters));
			}

			bool IsCurrentLineFullyRevealed() const
			{
				if (mCurrentImage >= mCutTextGroups.size() ||
					mCurrentTextLine >= mCutTextGroups[mCurrentImage].size())
					return true;

				const Entity textEntity = mCutTextGroups[mCurrentImage][mCurrentTextLine];
				if (!p_ecs.HaveComponent<Text>(textEntity))
					return true;

				return p_ecs.GetComponent<Text>(textEntity).GetContent() ==
					mOriginalTextContent[mCurrentImage][mCurrentTextLine];
			}

			void RevealCurrentLineFully()
			{
				if (mCurrentImage >= mCutTextGroups.size() ||
					mCurrentTextLine >= mCutTextGroups[mCurrentImage].size())
					return;

				Entity textEntity = mCutTextGroups[mCurrentImage][mCurrentTextLine];
				if (!p_ecs.HaveComponent<Text>(textEntity))
					return;

				p_ecs.GetComponent<Text>(textEntity).SetContent(
					mOriginalTextContent[mCurrentImage][mCurrentTextLine]);
			}

			bool TryAdvanceOverlayVideo()
			{
				if (mCurrentImage >= mCutOverlayGroups.size())
					return false;

				bool sawFinishedOverlayVideo = false;
				for (Entity overlayEntity : mCutOverlayGroups[mCurrentImage])
				{
					if (!p_ecs.HaveComponent<Video>(overlayEntity))
						continue;

					auto& overlayVideo = p_ecs.GetComponent<Video>(overlayEntity);
					if (!overlayVideo.finished)
						return false;

					sawFinishedOverlayVideo = true;
				}

				return sawFinishedOverlayVideo && AdvanceCutScene();
			}

			bool HasPendingOverlayVideo() const
			{
				if (mCurrentImage >= mCutOverlayGroups.size())
					return false;

				for (Entity overlayEntity : mCutOverlayGroups[mCurrentImage])
				{
					if (!p_ecs.HaveComponent<Video>(overlayEntity))
						continue;

					if (!p_ecs.GetComponent<Video>(overlayEntity).finished)
						return true;
				}

				return false;
			}

			std::unique_ptr<InputSystem>& mInput;
			std::vector<Entity> mCutImages{};
			std::vector<vec2D> mOriginalScales{};
			std::vector<std::vector<Entity>> mCutOverlayGroups{};
			std::vector<std::vector<vec2D>> mOriginalOverlayScales{};
			std::vector<std::vector<Entity>> mCutTextGroups{};
			std::vector<std::vector<float>> mOriginalTextAlpha{};
			std::vector<std::vector<std::string>> mOriginalTextContent{};
			std::vector<bool> mOriginalVideoEnabled{};
			std::vector<bool> mOriginalVideoSkipOnMouseClick{};
			std::vector<std::vector<bool>> mOriginalOverlayVideoEnabled{};
			std::vector<std::vector<bool>> mOriginalOverlayVideoSkipOnMouseClick{};
			float mTypewriterElapsed{ 0.0f };
			float mTypewriterCharactersPerSecond{ 45.0f };
			size_t mCurrentImage{ 0 };
			size_t mCurrentTextLine{ 0 };
			Entity mNextButton{ 0 };
		};
	}
