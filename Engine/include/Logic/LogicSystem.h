

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