/*!------------------------------------------------------------------------------------------------
    File: System.h
    Project: NocturnalStudio

	Brief:
		This file contains the System class declaration
		which provides the base class for all systems within the engine.
		It is an interface class for Systems to be used to create additional systems

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the System.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Core/Core.h"
#include "ECS/Types.h"
namespace NocturnalEngine {
	template class NOCTURNAL_ENGINE_API std::set<Entity>;
	
	class NOCTURNAL_ENGINE_API System
	{
	public:
		/*!*************************************************************************
		Container for all entities within this system
		****************************************************************************/
		std::set<Entity> mEntities;
		/*!*************************************************************************
		Returns name of System
		****************************************************************************/
		virtual std::string GetName() = 0;
		virtual ~System() {};
		//System function
		virtual void Init() = 0 {};
		virtual void Update(float Frametime) { UNREFERENCED_PARAMETER(Frametime); };
		virtual void End() = 0 {};

	protected:
		 std::string m_DebugName;

	};
}