
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