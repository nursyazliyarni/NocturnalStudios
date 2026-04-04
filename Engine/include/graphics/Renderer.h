/*!------------------------------------------------------------------------------------------------
    File: Renderer.h
    Project: NocturnalStudio

	Brief:
		This file contain the declaration for Renderer which contains all necessary draw functions
		for the engine.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Renderer.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "ResourceManager/ResourceManager.h"
#include "SpriteRendering.h"
#include "Camera2D.h"
#include "Math/Math.h"

namespace NocturnalEngine {

	class Renderer
	{
	public:
		void SetClearColor(const Vec4& color);
		void Clear();

		//render call
		static void Init();
		static void ShutDown();

		static void Begin(Camera2D& camera);
		static void End();
		static void Flush();
		

		static void DrawIndexed(const MultiRefs<VertexArray>& vertexarray, unsigned int IndexCount = 0);
		static void DrawForLine(const MultiRefs<VertexArray>& vertexarray, unsigned int vertexCount);

		void DrawQuad(const vec2D& position, const vec2D& size, const Vec4& color);
		void DrawQuad(const Vec3& position, const vec2D& size, const Vec4& color);
		void DrawQuad(const Mat4x4& transform, const Vec4& color);
		void DrawQuadImpact(const Vec3& position, const vec2D& size, const Vec4& color);
		void DrawQuad(const vec2D& position, const vec2D& size, const MultiRefs<Texture>& texture);
		void DrawQuad(const Vec3& position, const vec2D& size, const MultiRefs<Texture>& texture, const Vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawQuad(const Mat4x4& transform, const MultiRefs<Texture>& texture, const Vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
		
		void DrawQuad(const vec2D& position, const vec2D& size, float rotation, const Vec4& color);
		void DrawQuad(const Vec3& position, const vec2D& size, float rotation, const Vec4& color);
		void DrawQuad(const vec2D& position, const vec2D& size, float rotation, const MultiRefs<Texture>& texture, const Vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawQuad(const Vec3& position, const vec2D& size, float rotation, const MultiRefs<Texture>& texture, const Vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

		//handle sprite draws
		void DrawSprite(const vec2D& position, const vec2D& size, const float& rotation, const MultiRefs<SpriteRender>& sprite);
		void DrawSprite(const Vec3& position, const vec2D& size, const float& rotation, const MultiRefs<SpriteRender>& sprite);
		void DrawSprite(const Mat4x4& transform, const MultiRefs<SpriteRender>& sprite);



		void DrawLine(const Vec3& position0, const Vec3& position1, const Vec4& color);
		void DrawDashedLine(const Vec3& point_1, const Vec3& point_2, const Vec4& color, float offset, float line_length,
	    float line_gap);
		void DrawRect(const Vec3& position, const vec2D& size, const Vec4& color);
		void DrawDashedRect(const Vec3& position, const vec2D& size, const Vec4& color, float offset, float line_length,
	    float line_gap);
		void DrawCircle(const Mat4x4& transform, const Vec4& color, float depth = 0.01f, float decline = 0.01f);


		//Information for renderer
		struct Information
		{
			unsigned int n_DrawCalls = 0; 
			unsigned int n_Quad = 0;

			unsigned int TotalVertexUsed()const { return n_Quad * 4; }
			unsigned int TotalIndexUsed()const { return n_Quad * 6; }
		};
		void ResetInfo();
		static Information GetInfo();
	private:

		struct SceneData
		{
			Mat4x4 ViewProjectionMatrix;
		};
		static Singleton<SceneData> s_SceneData;

	private:
		static void StartBatch();
		static void NextBatch();
	};
}