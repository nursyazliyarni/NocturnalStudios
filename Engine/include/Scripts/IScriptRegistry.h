/*!------------------------------------------------------------------------------------------------
    File: IScriptRegistry.h
    Project: NocturnalStudio

    Brief:
        This file enables injection of scripts from an instance of a game running on
        NocturnalEngine into the Engine itself. This allows for the user to not need to 
        hardcode in values when loading in scripts.

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the IScriptRegistry class and its rerspective functions


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "ECS/ECS.h"
#pragma once
namespace NocturnalEngine {

    /*!*************************************************************************
    The IScriptRegistry is used by the Logic Component to attach and manage
    script that are defined by the Game creator
    ****************************************************************************/
    class IScriptRegistry {
    public:
        /*!*************************************************************************
        Destructor
        ****************************************************************************/
        virtual ~IScriptRegistry() = default;

        /*!*************************************************************************
        Returns a vector containing the names of all the scripts
        ****************************************************************************/
        virtual std::vector<std::string> GetAvailableScripts() const = 0;

        /*!*************************************************************************
        Check if the script can be attached to the entity. Each script has its own
        predefined requirements (some need specific components) so based on the 
        name and the provided entity id the registry will check
        ****************************************************************************/
        virtual bool CanAttachScript(const std::string& name, Entity entity) const = 0;

        /*!*************************************************************************
        Creates an instance of the script and returns the pointer to it
        ****************************************************************************/
        virtual IScript* CreateScript(const std::string& name) = 0;
    };
}
