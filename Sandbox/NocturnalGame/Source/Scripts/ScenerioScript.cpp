/*!------------------------------------------------------------------------------------------------
    File: ScenerioScript.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the ScenerioScript script class which is responsible 
        for managing scenario-specific logic within the engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the ScenerioScript class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/SceneManager.h"
#include "Scripts/ScenerioScript.h"


namespace NocturnalEngine {
	/*!*************************************************************************
	Default constructor for the scenario script
	****************************************************************************/
	ScenerioScript::ScenerioScript() : SelectedScene{""} {}
		/*!*************************************************************************
		Start state for the scenario script
		****************************************************************************/
		void ScenerioScript::Start(){}
		/*!*************************************************************************
		Update state for the scenario script
		****************************************************************************/
		void ScenerioScript::Update(float Frametime)
		{
			UNREFERENCED_PARAMETER(Frametime);
		}
		/*!*************************************************************************
		End state for the scenario script
		****************************************************************************/
		void ScenerioScript::End()
		{
			
		}

		/*!*************************************************************************
		Returns a clone of the scenario script
		****************************************************************************/
		IScript* ScenerioScript::Clone() const
		{
			return new ScenerioScript(*this);
		}
		/*!*************************************************************************
		Returns the name of the scenario script
		****************************************************************************/
		std::string ScenerioScript::GetScriptName()
		{
			return "ScenerioScript";
		}


}