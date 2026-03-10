/*!------------------------------------------------------------------------------------------------
    File: GameComponents.h
    Project: NocturnalStudio

    Brief:
        This file contains the gameside components and defines a macro (?) for them. IDK if its 
        actually being used tho but just to be safe. Its also contains the includes for the 
        components itself so easy to refer to 

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - 

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "AnimalAttributes.h"
#include "ItemsAttributes.h"
#include "Tutorial_Attributes.h"

#define C_AnimalAttributesComponent NocturnalEngine::AnimalAttributes::AnimalAttributes()
#define C_ItemsAttributesComponent NocturnalEngine::ItemsAttributes::ItemsAttributes()
#define C_TutorialAttributesComponent NocturnalEngine::TutorialAttributes::TutorialAttributes()