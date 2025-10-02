
#pragma once //Makes sure this header is only included once
#include "../math/Matrix.h"
#include "../math/Vector.h"

#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

namespace Debug
{
	class Drawer 
	{
	public:
		void DebugDrawRectangle();
		void DebugDrawCircle();
	};
} 

#endif