/*!------------------------------------------------------------------------------------------------
    File: Components.cpp
    Project: NocturnalStudio

    Brief:


    Primary Author:
         LAU XIANG YANG (100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"
#define p_ECompReg NocturnalEngine::EngineComponentRegistry::GetInstance()

namespace NocturnalEngine {
    class EngineComponentRegistry :public ComponentRegistry {
    public:
        static EngineComponentRegistry& GetInstance() {
            static EngineComponentRegistry instance;
            return instance;
        }

        void RegisterAllEngineComponents();
    private:
        EngineComponentRegistry() = default;

    };
}