/*!------------------------------------------------------------------------------------------------
    File: ButtonResponse.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the ButtonResponse script class which is responsible 
        for managing button responses within the engine.
	
    Primary Author:
         LAU XIANG YANG (50%)
		- Created the ButtonResponse class and its function specification.
	Seconday Author(s):
		LAYCOCK ENHUI SARAH (50%)
		- Added random sound functionality for specific ingame buttons so 
		  that they play a random sound instead of one singular sound
		- updated the collision logic so it will actually work with the colliders
		  and and accept circle colliders
		(06/03/2026)
		- Rewrote the script so none of the values hardcoded. Instead linked the script to
		  ButtonComponent which users can add the logic into on that side

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/ButtonResponse.h"
#include "ECS/SceneManager.h"
#include "GUI/GUI.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Scripts/GlobalGameManager.h"
#include "Math/Random.h"
#include "Scripts/TabButton.h"



namespace NocturnalEngine
{
	using ButtonFlag = ButtonComponent::ButtonFlag;


	void ButtonResponse::Start() {
	
		if (p_ecs.HaveComponent<Sprite>(entityID))
		{
			originalTexture = p_ecs.GetComponent<Sprite>(entityID).GetTexture();
		}
	}

	void ButtonResponse::Update(float Frametime)
	{

		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });// ensure that camera is set to  center at the start
		UNREFERENCED_PARAMETER(Frametime);


		if (KeyboardSceneSwitch()) return;


		if (!p_ecs.HaveComponent<ButtonComponent>(entityID)||
			!p_ecs.HaveComponent<Collider>(entityID)||
			!p_ecs.HaveComponent<Transform>(entityID)) return;

		ButtonComponent& buttonComponent{ p_ecs.GetComponent<ButtonComponent>(entityID) };
		int buttonFlag{ buttonComponent.Flag() };


		if (buttonFlag ==0) return;

		Collider& col{ p_ecs.GetComponent<Collider>(entityID) };
		Transform& transform{ p_ecs.GetComponent<Transform>(entityID) };

		if (p_GUI->Check_HTP() == false) // if HTP mode is off allow button to be click and interacted with
		{
			//this code is to check the current state of the mouse
			if (curr_state != button_state::click)curr_state = button_state::idle;// return to idle whe the state is set to release at the start of the next game loop


			if (p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && curr_state == button_state::idle)
			{
				curr_state = button_state::click; //if left mouse is held down set mouse state to click
			}

			else if (p_Input->MouseIsReleased(GLFW_MOUSE_BUTTON_LEFT) && curr_state == button_state::click)
			{
				curr_state = button_state::release;//when mouse key is release set state to release
			}
		}

		if (is_within_box(p_GUI->MousePosition, col, transform))//check if the system is hovering over an object, change to the hover sprite if it is
		{
			if (initial_hover)
			{
				initial_hover = false;
				leftcollider = false;
				entercollider = true;
			}

			if (entercollider)
			{
				if (buttonFlag & ButtonFlag::AUDIO_ON_HOVER) PlayAudio(true, buttonComponent);

				if (buttonFlag & ButtonFlag::TEX_CHANGE_ON_HOVER) ChangeTexture(buttonComponent.hoverTexture);

				if (buttonFlag & ButtonFlag::SCALE_ON_HOVER)ScaleButton(buttonComponent.hoverScale,transform);

				entercollider = false;
			}
			
			if (curr_state == button_state::release)// when mouse button is release, call the button functions, release is selected to prevent function from calling when mouse is held down 
			{
				// goes to a scene
				if (buttonFlag & ButtonFlag::GOTO_SCENE) p_Scene->setSceneToLoad(buttonComponent.gotoScene);

				// resets stats
				if (buttonFlag & ButtonFlag::RESET_STATS) p_GameManager.ResetSystem(true);

				// sets the game state
				if (buttonFlag & ButtonFlag::SET_GAME_STATE) p_GameManager.SetState(static_cast<GameManager::GameState>(buttonComponent.stateToSet));

				//calls the next level function
				if (buttonFlag & ButtonFlag::NEXT_LEVEL) p_GameManager.NextLevel();

				//sets volume to mute
				if (buttonFlag & ButtonFlag::MUTE_AUDIO) p_Audio->SetVolumeByChannel(p_Audio->GetMasterChannelGroup(), 0.0f); //tell the system to mute

				//sets camera zoom
				if (buttonFlag & ButtonFlag::SET_CAMERA_ZOOM) Graphic::mcamera->SetZoomLevel(buttonComponent.cameraZoom);

				//unused, was for a prototype single-scene game
				if (buttonFlag & ButtonFlag::ALT_SCENE_SWITCH) p_GameManager.SetSceneToSwitch(buttonComponent.altSceneSwitch);

				//exits the game
				if (buttonFlag & ButtonFlag::EXIT_GAME) p_GUI->toggle_script();

			}

			if (curr_state == button_state::click) 
			{
				if (buttonFlag & ButtonFlag::AUDIO_ON_CLICK) PlayAudio(false, buttonComponent);

				if (buttonFlag & ButtonFlag::TEX_CHANGE_ON_CLICK) ChangeTexture(buttonComponent.clickTexture);
			}
		}
		else 
		{
			if (!leftcollider) 
			{
				//changes texture back
				if (buttonFlag & ButtonFlag::TEX_CHANGE_ON_HOVER) ChangeTexture(originalTexture);

				//unscales the button
				if (buttonFlag & ButtonFlag::SCALE_ON_HOVER)ScaleButton(1.f/buttonComponent.hoverScale, transform);

				//tells the audio to mute 
				if (buttonFlag & ButtonFlag::AUDIO_ON_HOVER)MuteAudio();

				leftcollider = true;
				initial_hover = true;
				entercollider = false;
			}
		}

		
	}

	void ButtonResponse::End()
	{

	}

	ButtonResponse* ButtonResponse::Clone() const
	{
		return new ButtonResponse(*this);
	}


	std::string ButtonResponse::GetScriptName()
	{
		return "ButtonResponse";
	}

	bool ButtonResponse::KeyboardSceneSwitch() {

		// ------------------------------------------------------------
		// Keyboard shortcuts for scene switching (works anywhere)
		// 1 / W -> Flamingo
		// 2 / A -> Seal
		// 3 / S -> Horse
		// 4 / D -> Owl
		// ------------------------------------------------------------	
		if (p_GUI->Check_HTP() == false) // don't allow shortcuts during HTP overlay
		{
			if (p_Input->KeyPressed(GLFW_KEY_1) || p_Input->KeyPressed(GLFW_KEY_W))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");
				return true;
			}
			if (p_Input->KeyPressed(GLFW_KEY_2) || p_Input->KeyPressed(GLFW_KEY_A))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/SealScene.json");
				return true;
			}
			if (p_Input->KeyPressed(GLFW_KEY_3) || p_Input->KeyPressed(GLFW_KEY_S))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/HorseScene.json");
				return true;
			}
			if (p_GameManager.CurrentLevel() > 1 && (p_Input->KeyPressed(GLFW_KEY_4) || p_Input->KeyPressed(GLFW_KEY_D)))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/OwlScene.json");
				return true;
			}
		}

		return false;
	}

	//im so tired i dont wanna explain this
	//thius just checks for an audio component attaxchedx tpo the button object
	//then depending on if itds hpvering ot clicking and if itsd random or not 
	//it plays the sound
	void ButtonResponse::PlayAudio(bool hover, const ButtonComponent& button) {
		if (!p_ecs.HaveComponent<Audio>(entityID)) return;

		Audio& audio{ p_ecs.GetComponent<Audio>(entityID) };
		int soundIndex{ 0 };
		if (hover)
		{
			if (button.hoverAudioRandom)soundIndex = Random::Range(0, audio.GetSize() - 1);
			else soundIndex = button.hoverAudioIndex;

			hoverAudioIndex = soundIndex;
		}
		else 
		{
			if (button.clickAudioRandom)soundIndex = Random::Range(0, audio.GetSize() - 1);
			else soundIndex = button.clickAudioIndex;
		}

		if (audio.GetArr().size() <= soundIndex)return;

		audio[soundIndex].should_play = true;
	}

	//this is used for hover because what if u want to keep on hearing the same sound effect this will stop the sound efect apon exiting
	void ButtonResponse::MuteAudio() {
		if (!p_ecs.HaveComponent<Audio>(entityID)) return;

		Audio& audio{ p_ecs.GetComponent<Audio>(entityID) };
		if (audio.GetArr().size() <= hoverAudioIndex)return;

		audio[hoverAudioIndex].should_stop = true;
	}

	//changes texture by right the button shouldf have a texture
	void ButtonResponse::ChangeTexture(const std::string& tex) {
		if (tex.empty())return;
		if (!p_ecs.HaveComponent<Sprite>(entityID)) return;

		p_ecs.GetComponent<Sprite>(entityID).SetTexture(tex);
		
	}

	//sca;es buton
	void ButtonResponse::ScaleButton(float scale, Transform& t) {

		t.ScaleBy(scale);
	}

	bool ButtonResponse::is_within_box(glm::vec2 cur, Collider box, Transform trans)//unique aabb code for entity and mouse
	{
		vec2D mPos{ cur.x,cur.y };
		for (int i = 0; i < 2 ; i++)
		{
			vec2D offsetPos{ trans.GetPos().x + box[i].mOffset.x ,trans.GetPos().y + box[i].mOffset.y };
			switch (box[i].mCol)
			{
			case Collider::ColliderType::circle:
			case Collider::ColliderType::bubble:
				if (distance(mPos, offsetPos) < box[i].mRadius) {
					return true;
				}
				break;
			case Collider::ColliderType::rect:
			case Collider::ColliderType::box:
				vec2D minPt{ offsetPos.x - box[i].mMin.x,offsetPos.y - box[i].mMin.y }, maxPt{ offsetPos.x + box[i].mMax.x,offsetPos.y + box[i].mMax.y };
				if (mPos.x > minPt.x && mPos.x<maxPt.x && mPos.y>minPt.y && mPos.y < maxPt.y)
				{
					return true;
					//std::cout << "entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " collider check\n";

				}
				break;
			}
		}
		return false;
	}
}