
#pragma once
#include "Graphics/Camera2D.h"
namespace NocturnalEngine {

	struct AABB
	{
		glm::vec2 min;
		glm::vec2 max;
	};

	struct Picker
	{
		static int Pick(Camera2D* camera, std::multimap<float, Transform*> scene);
	};
}