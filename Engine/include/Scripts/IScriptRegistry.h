/*!------------------------------------------------------------------------------------------------
    File: IScriptRegistry.h
    Project: NocturnalStudio

    Brief:
        This file enables injection of scripts into the leveleditor

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the IScriptRegistry class and its rerspective functoiond


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "ECS/ECS.h"
#pragma once
namespace NocturnalEngine {
    
    class IScriptRegistry {
    public:
        virtual ~IScriptRegistry() = default;

        virtual std::vector<std::string> GetAvailableScripts() const = 0;
        virtual bool CanAttachScript(const std::string& name, Entity entity) const = 0;
        virtual IScript* CreateScript(const std::string& name) = 0;
    };
}
