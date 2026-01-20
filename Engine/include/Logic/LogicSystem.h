/*!------------------------------------------------------------------------------------------------
    File: LogicSystem.h
    Project: NocturnalStudio

	Brief:
		Logic System used for iterating through each entity with a logic
		component, grabbing the vector of scripts from the logic components and
		iterating through each script to call its update functions
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the LogicSystem.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "System/System.h"
#include "ECS/Components/Components.h"
#include "ECS/ECS.h"

namespace NocturnalEngine
{

	class LogicSystem : public System
	{
	public:
		/*!*************************************************************************
		Returns string name of system
		****************************************************************************/
		virtual std::string GetName() { return "LogicSystem"; }
		/*!*************************************************************************
		Initialize Logic System
		****************************************************************************/
		virtual void Init() override;
		/*!*************************************************************************
		Update Loop for Logic System
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		End State for Logic System which will delete all script data
		****************************************************************************/
		virtual void End() override;
	};
}