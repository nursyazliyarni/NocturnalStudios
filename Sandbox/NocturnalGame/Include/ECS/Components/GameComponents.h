/*!------------------------------------------------------------------------------------------------
    File: Components.h
    Project: NocturnalStudio

    Brief:
        This file contains all the Various Components to be used in the ECS
        in the future. Currently, Every Component is still handling its own data
        by inheriting from an abstract class of IComponent to serialize and
        deserialize its data

    Primary Author:
         LAU XIANG YANG (95%)
        - Created the Components.h file and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added Text component integration into ECS component includes/macros.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "AnimalAttributes.h"
#include "ItemsAttributes.h"

#define C_AnimalAttributesComponent NocturnalEngine::AnimalAttributes::AnimalAttributes()
#define C_ItemsAttributesComponent NocturnalEngine::ItemsAttributes::ItemsAttributes(