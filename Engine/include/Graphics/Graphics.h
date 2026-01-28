/*!------------------------------------------------------------------------------------------------
    File: Graphics.h
    Project: NocturnalStudio

	Brief:
		This file contains the class declarations and definitions for graphic system.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Graphics.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "System/System.h"
#include "Graphics/Texture.h"
#include "ECS/Components/Components.h"
#include "Math/Vmath.h"
#include "Math/matrix3D.h"
#include "Math/physics.h"
#include "Camera2D.h"
#include "Window/Window.h"
#include "Renderer.h"
#include "Buffer.h"
#include "TextRendering.h"
#include "Math/math.h"
#include "Animation/Animation.h"
#include <array>

namespace NocturnalEngine {

	class Graphic : public System
	{
	public:
		
		//For Debugging Purposes
		virtual std::string GetName() { return "graphic"; }

		virtual void Init() override;
		virtual void Update(float Frametime) override;
		virtual void End() override;

		inline static Camera2D camera { -1.0f, 1.0f, -1.0f , 1.0f };
		inline static Camera2D scene_camera { -1.0f, 1.0f, -1.0f , 1.0f };
		inline static Camera2D *mcamera = nullptr;

		static constexpr int MAX_LAYERS = 10;
		inline static std::array<bool, MAX_LAYERS> layerEnabled{}; // all default false, we'll set true in Init

		bool misFps = false;
	private:
		//for testing
		std::unique_ptr<Renderer> mRenderer = std::make_unique<Renderer>();
		std::shared_ptr<Font> mFont = std::make_shared<Font>();
		MultiRefs<SpriteRender> mIndex1;
		WinData mWinData{};
		Animation mAimator;
	private:
		void LoadIconsTexture(std::string filename);
	};
}