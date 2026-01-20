/*!------------------------------------------------------------------------------------------------
    File: Camera.h
    Project: NocturnalStudio

	Brief:
		This file contains the class declarations and definitions for camera projection.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Camera.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <glm/glm.hpp>

namespace NocturnalEngine {

	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};

}