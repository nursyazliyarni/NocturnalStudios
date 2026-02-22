/*!------------------------------------------------------------------------------------------------
    File: EngineComponentReg.h
    Project: NocturnalStudio

    Brief: This contains the declaration of the engine specific component registry. The leveleditor
            and varioius other systems can use this to operate on components


    Primary Author:
         LAYCOCK ENHUI SARAH(100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ECS/ComponentRegistry.h"
#define p_ECompReg NocturnalEngine::EngineComponentRegistry::GetInstance()

namespace NocturnalEngine {
    /*!*************************************************************************
    The EngineComponentRegistry contains the components that the Engine uses.
    This contains functions that allow the editor to iterate through the list
    and display it
    ****************************************************************************/
    class EngineComponentRegistry :public ComponentRegistry {
    public:
        /*!*************************************************************************
        Singleton, returns a reference
        ****************************************************************************/
        static EngineComponentRegistry& GetInstance() {
            static EngineComponentRegistry instance;
            return instance;
        }

        /*!*************************************************************************
        Helper function to register all of the engine components in
        ****************************************************************************/
        void RegisterAllEngineComponents();
    private:

        /*!*************************************************************************
        Constructor. Uhh if the memory manager is enabled might make the component
        map reserve memory so it doesnt keep rehashing. Not sure should i do that by
        default. I havent coded it yet tho meow
        ****************************************************************************/
        EngineComponentRegistry() = default;

    };
}