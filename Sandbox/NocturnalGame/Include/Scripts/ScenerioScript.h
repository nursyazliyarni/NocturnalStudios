/*!------------------------------------------------------------------------------------------------
    File: ScenerioScript.h
    Project: NocturnalStudio

	Brief:
		 This file contains the ScenerioScript class declaration
		 which act as the Game scene manager when the game is being played

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the ScenerioScript.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/IScript.h"
#include "ECS/Types.h"
#include "GLFW/glfw3.h"
#include "ECS/ECS.h"
#include "Input/Input.h"

namespace NocturnalEngine
{
	class ScenerioScript : public IScript
	{
	public:
		ScenerioScript();
		~ScenerioScript() = default;
		/*!*************************************************************************
		This function initialises the script. As there are no data members that require
		initialization, this function is empty
		****************************************************************************/
		virtual void Start() override;
		/*!*************************************************************************
		This function runs the logic of the script to check for collision and apply
		a counter-force to the entity's current velocity which is calculated based on
		the current velocity and the collision normal
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		This function ends the script by deleting the pointer to this script
		****************************************************************************/
		virtual void End() override;

		bool Requirements(Entity e) override {
			return true;
		}

		/*!*************************************************************************
		This function clones the script by allocating and constructing a copy of this
		script and returning it
		****************************************************************************/
		virtual IScript* Clone() const override;
		/*!*************************************************************************
		This function returns the purpose of the script as a flag for when it is called
		****************************************************************************/
		virtual std::string GetScriptName() override;

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		/*!*************************************************************************
		This Variable originally hold the name of the selected string
		****************************************************************************/
		std::string SelectedScene;
	};
}