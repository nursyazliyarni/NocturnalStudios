/*!------------------------------------------------------------------------------------------------
    File: ScriptFactory.h
    Project: NocturnalStudio

    Brief:
        This file enables injection of scripts into the leveleditor

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the ScriptFactory class and its rerspective functoiond


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
#include "Scripts/PainSpotSpawner.h"
#include "Scripts/EndScreenDisplay.h"
#include "Scripts/ToyHideAndRevealOnClick.h"

namespace NocturnalEngine {

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
            RegisterScriptType<PainSpotSpawner>();
            RegisterScriptType<EndScreenDisplay>();
            RegisterScriptType<ToyHideAndRevealOnClick>();
        

    }

       
}
