/*!------------------------------------------------------------------------------------------------
	File: KeyboardSceneSwitch.h
	Project: NocturnalStudio

	Brief:
		 This file contains the KeyboardSceneSwitch class declaration
		 which is responsible for managing the switching between scenes during gameplay
		 using the keyboard

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the KeyboardSceneSwitch.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#pragma once
#include "Scripts/IScript.h"


namespace NocturnalEngine {
	class KeyboardSceneSwitch : public IScript
	{
	public:

		KeyboardSceneSwitch()=default;
		~KeyboardSceneSwitch() = default;
		virtual KeyboardSceneSwitch* Clone() const override;
		virtual void Start() override;
		virtual void Update(float Frametime) override;
		virtual void End() override;

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Audio>(e))return true;
			return false;
		}

		std::vector<ScriptField> GetFields() override {
			return {
				{ "SceneName",    &sceneName    }
			};
		}
		virtual std::string GetScriptName();
		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		int GetRandomSoundIndex(const std::vector<Audio::AudioPiece>& a);

		std::string sceneName{};
	};
}