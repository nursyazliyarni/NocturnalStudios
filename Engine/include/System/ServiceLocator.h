/*!------------------------------------------------------------------------------------------------
    File: ServiceLocator.h
    Project: NocturnalStudio

    Brief:
        This file enables the engine to access various services from the Game side. 

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the ServiceLocator class and its rerspective functions


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
namespace NocturnalEngine{
    class IScriptRegistry; // Forward declare - scripts and their info
    class IGameConfig; // Game configurations
    class ComponentRegistry; //Game Components

    /*!*************************************************************************
    The ServiceLocator allows for the Engine to access some of the values from 
    the game side
    ****************************************************************************/
    class ServiceLocator {
    public:
        /*!*************************************************************************
        Singleton
        ****************************************************************************/
        static ServiceLocator& Get() {
            static ServiceLocator instance;
            return instance;
        }

        /*!*************************************************************************
        This is called by the game side to give the ServiceLocator a pointer to its
        script registry
        ****************************************************************************/
        void ProvideScriptRegistry( IScriptRegistry* registry) {
            m_scriptRegistry = registry;
        }

        /*!*************************************************************************
        This is called by the Engine side to fetch a pointer to the game's script 
        registry 
        MIGHT BE NULL, so please do check first before perfoming anything using this
        ****************************************************************************/
        IScriptRegistry* GetScriptRegistry() const {
            return m_scriptRegistry;
        }

        /*!*************************************************************************
        This is called by the game side to give the ServiceLocator a pointer to its
        config
        ****************************************************************************/
        void ProvideConfig(IGameConfig* config) {
            m_gameConfig = config;
        }

        /*!*************************************************************************
        This is called by the Engine side to fetch a pointer to the game's config
        MIGHT BE NULL, so please do check first before perfoming anything using this
        ****************************************************************************/
        IGameConfig* GetGameConfig() const {
            return m_gameConfig;
        }

        /*!*************************************************************************
        This is called by the game side to give the ServiceLocator a pointer to its
        component registry
        ****************************************************************************/
        void ProvideGameComponents(ComponentRegistry* comps) {
            m_gameComponents = comps;
        }

        /*!*************************************************************************
        This is called by the Engine side to fetch a pointer to the game's component registry. 
        MIGHT BE NULL, so please do check first before perfoming anything using this
        ****************************************************************************/
        ComponentRegistry* GetGameComponents() const {
            return m_gameComponents;
        }

    private:
        ServiceLocator() = default;
        IScriptRegistry* m_scriptRegistry = nullptr;
        IGameConfig* m_gameConfig = nullptr;
        ComponentRegistry* m_gameComponents = nullptr;
    };
}