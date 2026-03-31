/*!------------------------------------------------------------------------------------------------
    File: EngineCore.cpp
    Project: NocturnalStudio

	Brief:
		This file contains the implementation of the EngineCore class which 
		manages the main game loop and system initialization.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the EngineCore class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Core/EngineCore.h"
#include "Core/Log.h"
#include "Window/Window.h"
#include "LevelEditor/LevelEditor.h"
#include "Graphics/Graphics.h"
#include "Physics/CollisionSystem.h"
#include "Physics/PhysicsSystem.h"
#include "ECS/Components/Components.h"
#include "Timer/Time.h"
#include "Timer/Fps.h"
#include "ECS/ECS.h"
#include "ECS/EngineComponentReg.h"
#include "ECS/SceneManager.h"
#include "Audio/AudioEngine.h"
#include "Logic/LogicSystem.h"
#include "GUI/GUI.h"
#include "Core/MemoryManager.h"
#include "Video/VideoPlayback.h"
#include <stdlib.h>

namespace NocturnalEngine {
	bool end_state{ false }; //placeholder

/*!*************************************************************************
EngineCore constructor
****************************************************************************/
	EngineCore::EngineCore()
	{
		
		_set_abort_behavior(0, _CALL_REPORTFAULT);
#if !DEBUG
		ResourceManager::SetEngineResources();
#endif
		p_ecs.Init();
		p_ECompReg.RegisterAllEngineComponents();
		p_Scene->Init();
		FramePerSec::GetInstance().InitFrame();
	}

	/*!*************************************************************************
	EngineCore destructor
	****************************************************************************/
	EngineCore::~EngineCore()
	{
	}

	/*!*************************************************************************
	System input
	****************************************************************************/
	void EngineCore::SystemInput(System* system)
	{
		(void)system;
	}

	/*!*************************************************************************
	Run loop for EngineCore
	****************************************************************************/
	void EngineCore::Run()
	{
		Timer::GetInstance().GlobalTimeStarter();
		
		Window* m_window = new Window;
		m_window->Init();
		p_Audio->Init();


		if (m_GameLayer) {
			m_GameLayer->Init();
			m_GameLayer->RegisterScripts(); 
		}
		p_Editor->Init(m_window);

		auto mGraphics = p_ecs.RegisterSystem<Graphic>();
		{
			Signature signature;
			signature.set(p_ecs.GetComponentType<Transform>());
			signature.set(p_ecs.GetComponentType<Sprite>());
			p_ecs.SetSystemSignature<Graphic>(signature);
		}
		mGraphics->Init();

		auto mLogic = p_ecs.RegisterSystem<LogicSystem>();
		{
			Signature signature;
			signature.set(p_ecs.GetComponentType<Logic>());
			p_ecs.SetSystemSignature<LogicSystem>(signature);
		}
		mLogic->Init();

		auto mCollision = p_ecs.RegisterSystem<CollisionSystem>();
		{
			Signature signature;
			signature.set(p_ecs.GetComponentType<Transform>());
			signature.set(p_ecs.GetComponentType<RigidBody>());
			signature.set(p_ecs.GetComponentType<Collider>());
			p_ecs.SetSystemSignature<CollisionSystem>(signature);
		}
		mCollision->Init();

		auto mPosUpdate = p_ecs.RegisterSystem<PhysicsSystem>();
		{
			Signature signature;
			signature.set(p_ecs.GetComponentType<Transform>());
			signature.set(p_ecs.GetComponentType<RigidBody>());
			p_ecs.SetSystemSignature<PhysicsSystem>(signature);
		}
		mPosUpdate->Init();

		m_window->Show();

#if !DEBUG

#endif
		
		while (!glfwWindowShouldClose(m_window->GetWindow()) && !end_state) //engine loop
		{
			FramePerSec::GetInstance().StartFrameCount();
			Timer::GetInstance().Start(Systems::API);
			Timer::GetInstance().GetDT(Systems::API);

			
#if DEBUG	
			p_Editor->Update();
			p_Editor->Draw();

			if (p_Editor->is_ShowWindow == false && p_Input->KeyPressed(GLFW_KEY_0))
			{
				p_Editor->is_ShowWindow = true;
			}
#endif
			p_Scene->checkForSceneToLoad();

			if (p_Input->KeyPressed(GLFW_KEY_F9) && p_MemoryManager.IsEnabled())
			{
				p_MemoryManager.Shutdown();
				p_MemoryManager.Enable(!p_MemoryManager.IsEnabled());
				NOCTURNAL_ENGINE_INFO("Memory Manager Enabled: {}", p_MemoryManager.IsEnabled());
			}
			else if (p_Input->KeyPressed(GLFW_KEY_F9) && !p_MemoryManager.IsEnabled())
			{
				p_MemoryManager.Enable(!p_MemoryManager.IsEnabled());
				NOCTURNAL_ENGINE_INFO("Memory Manager Enabled: {}", p_MemoryManager.IsEnabled());
			}

			m_GameLayer->Update(Timer::GetInstance().GetGlobalDT());

			if (!m_window->isWindowNotFocus)
			{
				mLogic->Update(Timer::GetInstance().GetGlobalDT());
			}
			mCollision->Update(Timer::GetInstance().GetGlobalDT());
			mPosUpdate->Update();

			
			p_Audio->Update();

		
			end_state = p_GUI->Update(m_window);

			// test menu script

			p_Input->ResetPressedKey();//to fix the buggy error from glfwpollevent
			p_Input->ResetPressedMouse();


			m_window->Update(Timer::GetInstance().GetGlobalDT());
			p_Input->PollGamepad();
			mGraphics->Update(Timer::GetInstance().GetGlobalDT());

#if !DEBUG
			p_Editor->is_ShowWindow = false;
#endif 
			if (p_Editor->is_ShowWindow == false) {
				if(p_Input->IsGamepadConnected())m_window->SyncGamepadCursor();

			}
			FramePerSec::GetInstance().EndFrameCount();
			Timer::GetInstance().Update(Systems::API);
			if(!end_state)end_state = p_Editor->ShouldEnd();
		}
		mLogic->End();

		mGraphics->End();
		mCollision->End();
		mPosUpdate->End();

		End();

		delete m_window;
		m_window = nullptr;

		glfwTerminate();
	}

	/*!*************************************************************************
	End loop for EngineCore
	****************************************************************************/
	void EngineCore::End()
	{
		p_Editor->End();
		VideoPlayback::Shutdown();
		p_Audio->Release();
	}

	/*!*************************************************************************
	Create EngineCore
	****************************************************************************/
	EngineCore* CreateEngineCore()
	{
		return new EngineCore();
	}
}
