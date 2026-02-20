/*!------------------------------------------------------------------------------------------------
    File: GameComponentReg.cpp
    Project: NocturnalStudio

    Brief:


    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"

namespace NocturnalEngine {
    class GameComponentRegistry :public ComponentRegistry {
    public:
        static GameComponentRegistry* GetInstance() {
            static GameComponentRegistry instance;
            return &instance;
        }
        void RegisterAllGameComponents();

        template<typename T>
        void RegisterGameComponentInECS() {
            GetECS().template RegisterComponent<T>();
        }
    private:
        
        GameComponentRegistry() = default;
    };
}