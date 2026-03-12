/*!------------------------------------------------------------------------------------------------
    File: ScriptFactory.h
    Project: TEMPLATE_PROJECT_NAME

    Brief:
        This file enables injection of scripts into the leveleditor

    Primary Author:
        


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "Factories/ScriptFactory.h"


#include "Scripts/ScriptTemplate.h"


namespace NocturnalEngine {

    void ScriptFactory::RegisterAllGameScripts() {
        
            RegisterScriptType<ScriptTemplate>();
    }

       
}
