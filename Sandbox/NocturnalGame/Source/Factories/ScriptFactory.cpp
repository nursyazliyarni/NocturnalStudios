/*!------------------------------------------------------------------------------------------------
    File: ScriptFactory.cpp
    Project: NocturnalGame

    Brief:
        This file is where the user registers the scripts. These scripts will be used by the logic
        component in the engine to attach to entities

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the ScriptFactory class and its rerspective functions


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "Factories/ScriptFactory.h"


#include "Scripts/ButtonResponse.h"
#include "Scripts/ScenerioScript.h"
#include "Scripts/AudioManager.h"
#include "Scripts/BackgroundAudio.h"
#include "Scripts/SliderScript.h"
#include "Scripts/PauseMenu.h"
#include "Scripts/CutScene.h"
#include "Scripts/DialogueManager.h"
#include "Scripts/DragAndDrop.h"
#include "Scripts/HealthBar.h"
#include "Scripts/AnimalLogic.h"
#include "Scripts/TimerDisplay.h"
#include "Scripts/DigeticLog.h"
#include "Scripts/RevealTargetOnClick.h"
#include "Scripts/PainSpotDuplicateSpawner.h"
#include "Scripts/EndScreenDisplay.h"
#include "Scripts/ToyHideAndRevealOnClick.h"
#include "Scripts/TutorialLogic.h"
#include "Scripts/EncryptionKey.h"
#include "Scripts/KeysCollectedDisplay.h"
#include "Scripts/KeyboardSceneSwitch.h"
#include "Scripts/CheatPanel.h"


namespace NocturnalEngine {

    /*!*************************************************************************
    Registers all the game scripts in the script registry
    ****************************************************************************/
    void ScriptFactory::RegisterAllGameScripts() {
        

            RegisterScriptType<ButtonResponse>();
            RegisterScriptType<ScenerioScript>();
            RegisterScriptType<AudioManager>();
            RegisterScriptType<BackgroundAudio>();
            RegisterScriptType<SliderScript>();
            RegisterScriptType<PauseMenu>();
            RegisterScriptType<CutScene>();
            RegisterScriptType<DialogueManager>();
            RegisterScriptType<DragAndDrop>();
            RegisterScriptType<HealthBar>();
            RegisterScriptType<AnimalLogic>();
            RegisterScriptType<TimerDisplay>();
            RegisterScriptType<DigeticLog>();
            RegisterScriptType<RevealTargetOnClick>();
            RegisterScriptType<PainSpotDuplicateSpawner>();
            RegisterScriptType<EndScreenDisplay>();
            RegisterScriptType<ToyHideAndRevealOnClick>();
            RegisterScriptType<TutorialLogic>();
            RegisterScriptType<EncryptionKey>();
            RegisterScriptType<KeysCollectedDisplay>();
            RegisterScriptType<KeyboardSceneSwitch>();
            RegisterScriptType<CheatPanel>();

        

    }

       
}
