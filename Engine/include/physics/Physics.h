/*!------------------------------------------------------------------------------------------------
    File: Physics.h
    Project: NocturnalStudio

     This file contains the physics/runtime transform utilities for sprites:
    - Selection routing (which sprite or ALL is under control)
    - Per-frame movement integration (WASD) with simple AABB blocking
    - Discrete scaling steps with min/max clamping and center-preserving resize
    - Toggleable continuous rotation (J/K) plus reset (X)

    Primary Author:
		 Yu Xuan (%)
		-Brief

	Rishika Bimal Attodi:
		-Added scaling and rotation systems (limits, toggles, reset), and selection handshake
          with the input layer.


		Celeste

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