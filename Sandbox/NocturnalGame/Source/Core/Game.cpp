/*!------------------------------------------------------------------------------------------------
    File: Game.h
    Project: NocturnalStudio

    Brief:
        This file contains the declaration of EngineCore class.

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the EngineCore class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "Core/Game.h"
#include "System/ServiceLocator.h"
namespace NocturnalEngine {
    
    void NocturnalGameLayer::Init() {
        m_gameConfig = LuminaraZoo();
        ServiceLocator::Get().ProvideConfig(std::make_shared<LuminaraZoo>(m_gameConfig));
    }
    void NocturnalGameLayer::Update(float dt) {
        std::cout << "S";
    }
    void NocturnalGameLayer::Shutdown() {
        std::cout << "S";
    }

    void NocturnalGameLayer::RegisterScripts() {
        // Register all scripts
        m_scriptFactory.RegisterAllGameScripts();

        // Provide to engine
        ServiceLocator::Get().ProvideScriptRegistry(std::make_shared<ScriptFactory>(m_scriptFactory));


    }
    
}