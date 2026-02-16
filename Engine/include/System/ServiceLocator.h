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

    class ServiceLocator {
    public:
        static ServiceLocator& Get() {
            static ServiceLocator instance;
            return instance;
        }

        void ProvideScriptRegistry(std::shared_ptr < IScriptRegistry> registry) {
            m_scriptRegistry = registry;
        }

        std::shared_ptr < IScriptRegistry> GetScriptRegistry() const {
            return m_scriptRegistry;
        }

        void ProvideConfig(std::shared_ptr < IGameConfig> config) {
            m_gameConfig = config;
        }

        std::shared_ptr < IGameConfig> GetGameConfig() const {
            return m_gameConfig;
        }

    private:
        ServiceLocator() = default;
        std::shared_ptr < IScriptRegistry> m_scriptRegistry = nullptr;
        std::shared_ptr < IGameConfig> m_gameConfig = nullptr;
    };
}