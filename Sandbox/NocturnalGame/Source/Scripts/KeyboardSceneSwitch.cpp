/*!------------------------------------------------------------------------------------------------
	File: KeyboardSceneSwitch.cpp
	Project: NocturnalStudio

	Brief:
		This file contains the implementation of the KeyboardSceneSwitch script class which is responsible
		for managing background audio playback within the engine.

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the KeyboardSceneSwitch class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/KeyboardSceneSwitch.h"
#include "Audio/AudioEngine.h"
#include "ECS/Components/Audio.h"
#include "GUI/GUI.h"
#include "GLFW/glfw3.h"
#include "Input/Input.h"
#include "Math/Random.h"
#include "ECS/SceneManager.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	KeyboardSceneSwitch* KeyboardSceneSwitch::Clone() const
	{
		return new KeyboardSceneSwitch(*this);
	}

	void KeyboardSceneSwitch::Start()
	{


	}

	void KeyboardSceneSwitch::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);

		if (!p_ecs.HaveComponent<Audio>(GetScriptEntityID())) return;

		Audio &a { p_ecs.GetComponent<Audio>(GetScriptEntityID()) };


		// ------------------------------------------------------------
		// Keyboard shortcuts for scene switching (works anywhere)
		// 1 / W -> Flamingo
		// 2 / A -> Seal
		// 3 / S -> Horse
		// 4 / D -> Owl
		// ------------------------------------------------------------	
		if (p_GUI->Check_HTP() == false && p_Scene->isGameplay) // don't allow shortcuts during HTP overlay
		{
			if (p_Input->KeyPressed(GLFW_KEY_1) || p_Input->KeyPressed(GLFW_KEY_W)||p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP))
			{
				std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
				if (!ar.empty()) a[GetRandomSoundIndex(ar)].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");
				return;
			}
			if (p_Input->KeyPressed(GLFW_KEY_2) || p_Input->KeyPressed(GLFW_KEY_A) || p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT))
			{
				std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
				if (!ar.empty()) a[GetRandomSoundIndex(ar)].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/SealScene.json");
				return;
			}
			if (p_Input->KeyPressed(GLFW_KEY_3) || p_Input->KeyPressed(GLFW_KEY_S) || p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN))
			{
				std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
				if (!ar.empty()) a[GetRandomSoundIndex(ar)].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/HorseScene.json");
				return;
			}
			if (p_GameManager.CurrentLevel() > 1 && (p_Input->KeyPressed(GLFW_KEY_4) || p_Input->KeyPressed(GLFW_KEY_D) || p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)))
			{
				std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
				if (!ar.empty())a[GetRandomSoundIndex(ar)].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/OwlScene.json");
				return;
			}
			
			if (p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER)) {
				std::string sceneToGo{ p_GameManager.CanMoveScene(sceneName,false) };
				if (sceneToGo!="")
				{
					std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
					if (!ar.empty())a[GetRandomSoundIndex(ar)].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/"+sceneToGo+"Scene.json");
					return;
				}
			}
			if (p_Input->GamepadPressed(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER)) {
				std::string sceneToGo{ p_GameManager.CanMoveScene(sceneName,true) };
				if (sceneToGo != "")
				{
					std::vector<Audio::AudioPiece>& ar{ a.GetArr() };
					if (!ar.empty())a[GetRandomSoundIndex(ar)].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/" + sceneToGo + "Scene.json");
					return;
				}
			}
		}
	}

	void KeyboardSceneSwitch::End()
	{


	}

	int KeyboardSceneSwitch::GetRandomSoundIndex(const std::vector<Audio::AudioPiece>& a) {
		if (a.size() <= 1) return 0; // only BGM or empty, fall back to 0
		int maxIndex = static_cast<int>(a.size()) - 1;
		return Random::Range(1, maxIndex);
	}

	std::string KeyboardSceneSwitch::GetScriptName()
	{
		return "KeyboardSceneSwitch";
	}

}