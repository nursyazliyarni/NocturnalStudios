
#pragma once

#include "Core/Core.h"
#include "Core/Log.h"
#include "System/SystemManager.h"
#include "ECS/ECS.h"

namespace NocturnalEngine {
	class NOCTURNAL_ENGINE_API Application
	{

	public:
		enum class state
		{
			pause,
			play,
			quit
		};
		
		Application();
		virtual ~Application(); // will be override
		void SystemInput(System* system);
		void Run();				//run the application 
		void End();

	private:
	};
	 
	Application* CreateApplication();
}

