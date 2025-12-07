/*!------------------------------------------------------------------------------------------------
    File: Physics.h
    Project: NocturnalStudio

     This file contains the physics/runtime transform utilities for sprites:
    - Selection routing (which sprite or ALL is under control)
    - Per-frame movement integration (WASD) with simple AABB blocking
    - Discrete scaling steps with min/max clamping and center-preserving resize
    - Toggleable continuous rotation (J/K) plus reset (X)

     This file contains all the behaviour of our sprite when the correct input key is detected

    Primary Author:
         Name (%)
        -Brief
         Ang Yu Xuan (60%)
        - implement functions for movement, scaling and rotation for our sprite.

    Secondary Author:
		 Name (%)
        -Brief
        Rishika Bimal Attodi (33%)
        - upgrade the scaling and rotation function.

    Secondary Author:
        Chang Qi Xuan Celest (7%)
        - Implemented stop movement on collision in update_sprite_movement.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// integration, gravity, movement
#pragma once
#include <vector>
#include "graphics/Window.h"
#include "graphics/Renderer.h"
#include "input/Input.h"

#define UNREFERENCED_PARAMETER(P) (void)(P)

class Physics {
public:
	Physics() = default;
	
	int take_control(Renderer& render_obj, Input& input_obj);
	void update_sprite_movement(Renderer& render_obj, Input& input_obj);
	void update_sprite_scaling(Renderer& render_obj, Input& input_obj);
	void update_sprite_rotation(Renderer& render_obj, Input& input_obj);

private:

};