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
#include "Input/Input.h"

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
		CutScene() : timer(0), mScenePicker{ 0 }, mSwitchAudio{ 1 } {}
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
			for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i)
			{
				if (p_ecs.GetComponent<NameTag>(i).GetNameTag() == "CutScene Controller")
				{
					NocturnalEngine::Graphic::mcamera->SetPosition({ 0.0f, 0.0f, 0.0f });
				}
			}
			if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > mSwitchAudio))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[mSwitchAudio].should_play = true;
			}
		}
		/*!*************************************************************************
		Update Loop of Cutscenes Script
		****************************************************************************/
		virtual void Update(float Frametime) override
		{
			timer += Frametime;
			if (p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				++mScenePicker;
				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > mSwitchAudio))
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[mSwitchAudio].should_stop = true;
				}
				++mSwitchAudio;
				NocturnalEngine::Graphic::mcamera->SetPosition({ p_ecs.GetComponent<Transform>(mScenePicker).GetPos().x
					, p_ecs.GetComponent<Transform>(mScenePicker).GetPos().y, 0.0f });
				p_Input->mMouseStatus[GLFW_MOUSE_BUTTON_LEFT] = GLFW_RELEASE;
				timer = 0.0f;
				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > 10))
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[10].should_play = true;
				}

				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > mSwitchAudio))
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[mSwitchAudio].should_play = true;
				}

			}
			if (timer >= 5.0f)
			{
				++mScenePicker;
				++mSwitchAudio;
				NocturnalEngine::Graphic::mcamera->SetPosition({ p_ecs.GetComponent<Transform>(mScenePicker).GetPos().x
					, p_ecs.GetComponent<Transform>(mScenePicker).GetPos().y, 0.0f });
				timer = 0.0f;
				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > mSwitchAudio))
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[mSwitchAudio].should_play = true; //cutscene audio
				}
			}

			if (mScenePicker == 9)
			{
				//transit to Game scene
				p_Scene->setSceneToLoad("Assets/Scene/Game.json");
			}



		}
		/*!*************************************************************************
		End State for Cutscenes
		****************************************************************************/
		virtual void End() override
		{
			
		}

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Sprite>(e))return true;
			return false;
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
		float timer;
		Entity mScenePicker;
		int mSwitchAudio;
	};
}