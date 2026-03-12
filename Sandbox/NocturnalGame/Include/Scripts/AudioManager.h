/*!------------------------------------------------------------------------------------------------
    File: AudioManager.h
    Project: NocturnalStudio

	Brief:
		This file contains the AudioManager class declaration
		which is responsible for managing audio-related functionalities
		such as background music and sound effects within the engine.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the AudioManager.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Scripts/IScript.h"
#include "GLFW/glfw3.h"
#include "Input/Input.h"
#include "Math/physics.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Audio Manager Script
	****************************************************************************/
	class AudioManager : public IScript
	{
	public:
		/*!*************************************************************************
		Default constructor for Audio Manager
		****************************************************************************/
		AudioManager();
		/*!*************************************************************************
		Default destructor for HUD Controller
		****************************************************************************/
		~AudioManager() = default;
		/*!*************************************************************************
		Returns a new copy of Audio Manager Script
		****************************************************************************/
		virtual AudioManager* Clone() const override;
		/*!*************************************************************************
		Start State of Audio Manager Script
		****************************************************************************/
		virtual void Start() override;
		/*!*************************************************************************
		Update Loop of HUD Controller Script
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		End State for HUD Controller
		****************************************************************************/
		virtual void End() override;
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName();

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Audio>(e))return true;
			return false;
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }
		
	private:
		float BGMvol;
		float SFXvol;
		float Mastervol;

		
	};
}