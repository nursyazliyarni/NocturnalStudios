/*!------------------------------------------------------------------------------------------------
    File: Application.cpp
    Project: NocturnalStudio

	Brief:
		This file contains the implementation of the Application class which 
		manages the main game loop and system initialization.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the application class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Core/Application.h"
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
#include "ECS/SceneManager.h"
#include "Audio/AudioEngine.h"
#include "Scripts/ButtonResponse.h"
#include "Scripts/ScenerioScript.h"
#include "Scripts/SliderScript.h"
#include "Scripts/AudioManager.h"
#include "Logic/LogicSystem.h"
#include "Scripts/PauseMenu.h"
#include "GUI/GUI.h"
#include "Scripts/DialogueManager.h"
#include "Core/MemoryManager.h"
#include <stdlib.h>

namespace NocturnalEngine {
	bool end_state{ false }; //placeholder

/*!*************************************************************************
Application constructor
****************************************************************************/
	Application::Application()
	{
		_set_abort_behavior(0, _CALL_REPORTFAULT);
		p_ecs.Init();
		p_Scene->Init();
		FramePerSec::GetInstance().InitFrame();
	}

	/*!*************************************************************************
	Application destructor
	****************************************************************************/
	Application::~Application()
	{
	}

	/*!*************************************************************************
	System input
	****************************************************************************/
	void Application::SystemInput(System* system)
	{
		(void)system;
	}

	/*!*************************************************************************
	Run loop for application
	****************************************************************************/
	void Application::Run()
	{
		Timer::GetInstance().GlobalTimeStarter();
		
		Window* m_window = new Window;
		m_window->Init();
		p_Audio->Init();
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
#if !DEBUG
		// p_Scene->setSceneToLoad("Assets/Scene/SplashScreen.json");
		p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
#endif
		
		while (!glfwWindowShouldClose(m_window->GetWindow()) && end_state == false) //game loop
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

			if (p_Scene->isGameplay == true)
				glfwSetInputMode(m_window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(m_window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			if (!m_window->isWindowNotFocus)
			{
//#if !DEBUG
				mLogic->Update(Timer::GetInstance().GetGlobalDT());
//#endif
			}
			mCollision->Update(Timer::GetInstance().GetGlobalDT());
			mPosUpdate->Update();

			
			p_Audio->Update();

			p_Input->detect_draggable_item(GLFW_MOUSE_BUTTON_LEFT,p_Editor->is_ShowWindow);
			
			if (p_Input->KeyPressed(GLFW_KEY_F3))
			{
				for (Entity i = 0; i < p_ecs.GetTotalEntities(); ++i)
				{

				
					if (p_ecs.HaveComponent<Logic>(i) && p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager") != nullptr)
					{
						p_ecs.GetComponent<Logic>(i).GetScriptByName("DialogueManager")->mScriptPause = true;
					}

				}
			}
			for (Entity entity = 0; entity < p_ecs.GetTotalEntities(); entity++)
			{
				if (p_ecs.HaveComponent<NameTag>(entity) && p_ecs.GetComponent<NameTag>(entity).GetNameTag() == "Splash Screen")
				{
					if (p_ecs.GetComponent<Transform>(entity).GetScale().x > 3.5f)
						p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
				}
			}
			end_state = p_GUI->Update(m_window);

			// test menu script

			p_Input->ResetPressedKey();//to fix the buggy error from glfwpollevent
			p_Input->ResetPressedMouse();


			m_window->Update(Timer::GetInstance().GetGlobalDT());
			mGraphics->Update(Timer::GetInstance().GetGlobalDT());
			
#if !DEBUG
			p_Editor->is_ShowWindow = false;
#endif 
			FramePerSec::GetInstance().EndFrameCount();
			Timer::GetInstance().Update(Systems::API);
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
	End loop for application
	****************************************************************************/
	void Application::End()
	{
		p_Editor->End();
		p_Audio->Release();
	}

	/*!*************************************************************************
	Create application
	****************************************************************************/
	Application* CreateApplication()
	{
		return new Application();
	}
}