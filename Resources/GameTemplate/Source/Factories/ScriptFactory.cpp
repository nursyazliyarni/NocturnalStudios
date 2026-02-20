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


#include "Scripts/ScriptTemplate.h"


namespace NocturnalEngine {

    void ScriptFactory::RegisterAllGameScripts() {
        
            RegisterScriptType<ScriptTemplate>();
    }

       
}
