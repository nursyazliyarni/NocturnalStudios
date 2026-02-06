/*!------------------------------------------------------------------------------------------------
    File: PhysicsSystem.h
    Project: NocturnalStudio

	Brief:
		This file contains the system declaration for the physics system to update
		the entity's position. 
		
    Primary Author:
         Chang Qi Xuan Celest (100%)
		- Created the PhysicsSystem.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Math/Physics.h"
#include "ECS/Components/Components.h"
#include "pch.h"
#include "System/System.h"

namespace NocturnalEngine {
	class PhysicsSystem : public System
	{
	public:
		/*!*************************************************************************
		This function returns the purpose of the system as a flag for when it is called
		****************************************************************************/
		virtual std::string GetName() { return "Update Positions"; }
		/*!*************************************************************************
		This function initialises the system. As there are no data members that require
		initialization, this function is empty
		****************************************************************************/
		virtual void Init() override;
		/*!*************************************************************************
		This function runs the logics of the system to update the entity positions
		by setting the current position as the entity's next position which has been 
		calculated previously
		****************************************************************************/
		virtual void Update();
		/*!*************************************************************************
		This function ends the system. As there are no data members that require
		initialization, there are no data members that need to be un-initialised. 
		Therefore this function is empty
		****************************************************************************/
		void End();
	private:
    };
}