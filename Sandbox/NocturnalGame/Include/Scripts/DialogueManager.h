/*!------------------------------------------------------------------------------------------------
    File: DialogueManager.h
    Project: NocturnalStudio

	Brief:
		 This file contains the DialogueManager class declaration
		 which is responsible for managing dialogue functionalities within the engine 
		 for the dialogue components.

    Primary Author:
         CHANG QI XUAN CELEST (100%)
		- Created the DialogueManager.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/


#pragma once
#include "Scripts/IScript.h"
#include "Input/Input.h"
#include "GUI/GUI.h"
#include "Graphics/Camera2D.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Dialogue Manager Script
	****************************************************************************/
	class DialogueManager : public IScript
	{
	public:
		/*!*************************************************************************
		Default constructor for Dialogue Manager
		****************************************************************************/
		DialogueManager() : mDialogues{ "Dialogue1", "Dialogue2", "Dialogue3", "Dialogue4","Tutorial1","Tutorial2","Tutorial3",
										"Tutorial4","Tutorial5","Tutorial6","Dialogue7","Dialogue8","Dialogue9","Dialogue10",
										"Dialogue11","Dialogue12" },
			counter{ 0 }, MaxCounter{ 4 } {}
		/*!*************************************************************************
		Default destructor for Dialogue Manager
		****************************************************************************/
		~DialogueManager() = default;
		/*!*************************************************************************
		Returns a new copy of Dialogue Manager Script
		****************************************************************************/
		virtual DialogueManager* Clone() const override
		{
			return new DialogueManager(*this);
		}
		/*!*************************************************************************
		Start State of DialogueManager Script
		****************************************************************************/
		virtual void Start() override
		{
		}
		/*!*************************************************************************
		Update Loop of DialogueManager Script
		****************************************************************************/
		virtual void Update(float Frametime) override
		{

			UNREFERENCED_PARAMETER(Frametime);

		}
		/*!*************************************************************************
		End State for DialogueManager
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
			return "DialogueManager";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

		int counter;
	private:
		std::string mDialogues[16];
		int MaxCounter;
	};
}