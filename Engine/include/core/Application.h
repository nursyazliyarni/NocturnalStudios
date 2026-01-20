/*!------------------------------------------------------------------------------------------------
    File: Application.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration of Application class.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the application class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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

