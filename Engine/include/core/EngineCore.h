/*!------------------------------------------------------------------------------------------------
    File: EngineCore.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration of EngineCore class.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the EngineCore class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Core/Core.h"
#include "Core/Log.h"
#include "Core/IGameLayer.h"
#include "System/SystemManager.h"
#include "ECS/ECS.h"


namespace NocturnalEngine {
	class NOCTURNAL_ENGINE_API EngineCore
	{

	public:
		enum class state
		{
			pause,
			play,
			quit
		};
		
		EngineCore();
		virtual ~EngineCore(); // will be override
		void SystemInput(System* system);
		void Run();				//run the EngineCore 
		void End();

		void SetGameLayer(IGameLayer* mgame) { m_GameLayer = mgame; }

	private:
		IGameLayer* m_GameLayer;
	};
	 
	EngineCore* CreateEngineCore();
}

