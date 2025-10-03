/*!------------------------------------------------------------------------------------------------
    File: Physics.h
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
        -Brief

    Secondary Author:
		 Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// integration, gravity, movement
#pragma once
#include <vector>
#include "graphics/Window.h"
#include "graphics/Renderer.h"
#include "input/Input.h"

class Physics {
public:
	Physics() = default;
	
	int take_control(Renderer& render_obj, Input& input_obj);
	void update_sprite_movement(Renderer& render_obj, Input& input_obj);
	void update_sprite_scaling(Renderer& render_obj, Input& input_obj);
	void update_sprite_rotation(Renderer& render_obj, Input& input_obj);

private:

};