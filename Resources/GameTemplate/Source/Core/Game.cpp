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
#include "LevelEditor/LevelEditor.h"
#include "GUI/GUI.h"
#include "ECS/SceneManager.h"
#include "ECS/GameComponentReg.h"
namespace NocturnalEngine {
    
    void NocturnalGameLayer::Init() {
        m_gameConfig = NewGame();
        ServiceLocator::Get().ProvideConfig(&m_gameConfig);

        GameComponentRegistry::GetInstance()->RegisterAllGameComponents();
        ServiceLocator::Get().ProvideGameComponents(GameComponentRegistry::GetInstance());


#if !DEBUG
        p_Scene->setSceneToLoad("");
#endif
    }
    void NocturnalGameLayer::Update(float dt) {
        
        UNREFERENCED_PARAMETER(dt);
           
        
    }
    void NocturnalGameLayer::Shutdown() {
        
    }

    void NocturnalGameLayer::RegisterScripts() {
        // Register all scripts
        m_scriptFactory.RegisterAllGameScripts();

        // Provide to engine
        ServiceLocator::Get().ProvideScriptRegistry(&m_scriptFactory);


    }
    
}