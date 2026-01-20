/*!------------------------------------------------------------------------------------------------
    File: Camera2D.h
    Project: NocturnalStudio

	Brief:
		This file contains the class declarations and definitions for camera projection.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Camera2D.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "glm/glm.hpp" //for ortho, perspective and other matrices
#include "Window/Window.h"
#include "Timer/Time.h"
namespace NocturnalEngine {

	class Camera2D
	{
	public:
		Camera2D(float left, float right, float bottom, float top);//left, right, bottom and top

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return mPosition; }
		void SetPosition(const glm::vec3& position) { mPosition = position; RecalculateMatrix(); }
		void SetPositionSmooth(const float x, const float y, const float speed = 2.5f);
		void Follow(glm::vec2 pos);
		float GetRotation() const { return mRotation; }
		void SetRotation(float rotation) { mRotation = rotation; RecalculateMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return mProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const{ return mViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return mViewProjectionMatrix; }
		
		float GetZoomLevel()const { return mZoomLevel; }
		void SetZoomLevel(float level) { mZoomLevel = level; }
		void resetZoomLevel();

		bool MouseScrolling();

		void Resize(float width, float height);
	private:

		glm::mat4 mProjectionMatrix ;
		glm::mat4 mViewMatrix;
		glm::mat4 mViewProjectionMatrix;
		glm::vec3 mPosition;
		
		float mRotation = 0.0f;
		unsigned int mViewportWidth, mViewportHeight;
		float mAspectRatio = 0.0f;
		float mZoomLevel = 1.0f;
		Window mWindow;

	private:
		void RecalculateMatrix();
	};
}
