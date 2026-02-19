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
namespace NocturnalEngine{
    class IScriptRegistry; // Forward declare
    class IGameConfig;
    class ComponentRegistry;

    class ServiceLocator {
    public:
        static ServiceLocator& Get() {
            static ServiceLocator instance;
            return instance;
        }

        void ProvideScriptRegistry( IScriptRegistry* registry) {
            m_scriptRegistry = registry;
        }

        IScriptRegistry* GetScriptRegistry() const {
            return m_scriptRegistry;
        }

        void ProvideConfig(IGameConfig* config) {
            m_gameConfig = config;
        }

        IGameConfig* GetGameConfig() const {
            return m_gameConfig;
        }

        void ProvideGameComponents(ComponentRegistry* comps) {
            m_gameComponents = comps;
        }

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