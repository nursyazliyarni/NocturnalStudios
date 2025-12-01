
#pragma once
#include "Graphics/Renderer.h"
#include "Graphics/SpriteRendering.h"
#include "Math/Vmath.h"

namespace NocturnalEngine {

	class Animation
	{
	public:
	    static std::unordered_map<std::string, std::vector<float>> spriteContainer;// to store all animation info
		Animation();
		void UpdateAnimation(float deltatime, Sprite& sprite);
	private:
	};
}