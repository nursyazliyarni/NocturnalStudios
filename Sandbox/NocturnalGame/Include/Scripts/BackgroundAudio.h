/*!------------------------------------------------------------------------------------------------
    File: BackgroundAudio.h
    Project: NocturnalStudio

	Brief:
		 This file contains the BackgroundAudio class declaration
		 which is responsible for managing background audio functionalities within the engine.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the BackgroundAudio.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#pragma once
#include "Scripts/IScript.h"
#include "GLFW/glfw3.h"
#include "Input/Input.h"

namespace NocturnalEngine {
	class BackgroundAudio : public IScript
	{
	public:
		
		BackgroundAudio();
		~BackgroundAudio() = default;
		virtual BackgroundAudio* Clone() const override;
		virtual void Start() override;
		virtual void Update(float Frametime) override;
		virtual void End() override;

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Audio>(e))return true;
			return false;
		}
		virtual std::string GetScriptName();
		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		
	};
}