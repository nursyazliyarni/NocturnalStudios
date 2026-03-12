/*!------------------------------------------------------------------------------------------------
    File: GameComponentReg.h
    Project: NocturnalStudio

    Brief:  This file contains the class definition of the GameComponentRegistry. This is used by
            the engine to fetch information regarding any components declared on the game side 
            (in our case, we have two, they are initialized in GameComponents.cpp)

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"

namespace NocturnalEngine {

    /*!*************************************************************************
    The GameComponentRegistry contains the components unique to the game project
    and declared by the user. This is then passed in to the ServiceLocator so 
    that the editor can enable the user to modify them
    ****************************************************************************/
    class GameComponentRegistry :public ComponentRegistry {
    public:
        /*!*************************************************************************
        Singleton
        ****************************************************************************/
        static GameComponentRegistry* GetInstance() {
            static GameComponentRegistry instance;
            return &instance;
        }

        /*!*************************************************************************
        A simple helper function to register all game components. this is defined in
        GameComponents.cpp.
        ****************************************************************************/
        void RegisterAllGameComponents();

        /*!*************************************************************************
        I dunno why i added this ngl 
        ****************************************************************************/
        template<typename T>
        void RegisterGameComponentInECS() {
            GetECS().template RegisterComponent<T>();
        }
    private:
        
        GameComponentRegistry() = default;
    };
}