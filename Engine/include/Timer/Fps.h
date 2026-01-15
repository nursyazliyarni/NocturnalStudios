/*!------------------------------------------------------------------------------------------------
    File: Fps.h
    Project: NocturnalStudio

	Brief:
		This file contains a general implementation of a Frame Per Second (FPS) counter
		which helps in measuring and controlling the frame rate of the application.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Fps.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <GLFW/glfw3.h>
#include "Core/Core.h"

namespace NocturnalEngine {

	class NOCTURNAL_ENGINE_API FramePerSec
	{
	public:

		/*!*************************************************************************
		Init various frame counters
		****************************************************************************/
		void InitFrame() 
		{
			mCounter = 0.0f;
			mMaxFrameRate = 60.0f;
			mFrameRate = mMaxFrameRate;
			mFrameTime = 1 / mFrameRate;
			mMinFrameTime = 1 / mMaxFrameRate;
		}

		/*!*************************************************************************
		Start frame count using glfwGetTime()
		****************************************************************************/
		void StartFrameCount() { mStart = (float)glfwGetTime(); }

		/*!*************************************************************************
		End frame count
		****************************************************************************/
		float EndFrameCount()
		{
			while (((float)glfwGetTime() - mStart) < mMinFrameTime)
			{
				float end = (float)glfwGetTime();
				mFrameTime = end - mStart;
				mFps = 1.0f / mFrameTime;
			}
			
			return mFps;
		}

		/*!*************************************************************************
		Return fps
		****************************************************************************/
		float GetFps()const { return mFps; }
		static FramePerSec& GetInstance()
		{
			static FramePerSec instance;
			return instance;
		}
	private:
		float mCounter, mMaxFrameRate, mFrameRate, mFrameTime,
			mMinFrameTime, mStart, mFps;
	};

}