/*!------------------------------------------------------------------------------------------------
    File: DebugDraw.h
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

#pragma once 
#include "../math/Matrix.h"
#include "../math/Vector.h"

namespace Debug
{
	class Drawer
	{
	public:
		// Draws a rectangle given bottom-left position + width/height
		void DebugDrawRectangle(float x, float y, float w, float h, unsigned int color);

		// Draws a circle (for later use if needed)
		void DebugDrawCircle(float cx, float cy, float r, unsigned int color, int segments = 32);
	};
}
