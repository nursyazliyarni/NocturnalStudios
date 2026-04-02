/*!------------------------------------------------------------------------------------------------
    File: Window.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the Window script class which is responsible 
        for managing window creation and events within the engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Window class and its function specification.
		
    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Input/Input.h"
#include "LevelEditor/LevelEditor.h"
#include "Timer/Time.h"
#include "Window/Window.h"
#include <algorithm>

namespace NocturnalEngine{
	bool Window::isWindowNotFocus = false;

	Window::Window() : m_window{ nullptr }, m_monitor{ nullptr },
		m_windowData{ windowData.GetTitle(), windowData.GetWidth(), windowData.GetHeight(), windowData.GetCurrWidth(), windowData.GetCurrHeight(), 0, 0, 0, 0, false, 0.0, 0.0 },//should be serialized
		m_vsync{ false }, previousTime{glfwGetTime()}, frameCount{0}
	{
		windowData.DeserializeFromFile("Window.json");
		m_windowData.Title = windowData.GetTitle();
		m_windowData.m_Width = windowData.GetWidth();
		m_windowData.m_Height = windowData.GetHeight();
		m_windowData.m_CurrentWidth = windowData.GetCurrWidth();
		m_windowData.m_CurrentHeight = windowData.GetCurrHeight();
		m_windowData.m_WindowWidth = m_windowData.m_Width;
		m_windowData.m_WindowHeight = m_windowData.m_Height;
		m_windowData.m_ViewportX = 0;
		m_windowData.m_ViewportY = 0;
		windowData.SerializeToFile("Window.json");
	};

	/*!*************************************************************************
	Init loop of window, uses glfw to set and create window
	****************************************************************************/
	void Window::Init()
	{

		/* Initialize the library */
		if (!glfwInit())
		{
			NOCTURNAL_ENGINE_ASSERT(!glfwInit(), "\aGLFW init has failed - exit project...");
		}
		glfwSetErrorCallback(ErrorCallback);


		//specify the constraints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RED_BITS, 8); glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8); glfwWindowHint(GLFW_ALPHA_BITS, 8);
		//we are setting window size able toggle
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		//fullscreen
		m_monitor = glfwGetPrimaryMonitor();

		if (m_monitor)
		{
			NOCTURNAL_ENGINE_INFO("Primary monitor detected: {0}", glfwGetMonitorName(m_monitor));
		}

		if (m_windowData.m_CurrentWidth == 0)
		{
			m_windowData.m_CurrentWidth = m_windowData.m_Width;
		}
		if (m_windowData.m_CurrentHeight == 0)
		{
			m_windowData.m_CurrentHeight = m_windowData.m_Height;
		}

		std::string winTitle = m_windowData.Title;
#if DEBUG
		winTitle = "[NocturnalEngine] Editing " + winTitle;
#endif

#if !DEBUG
		const GLFWvidmode* mode = m_monitor ? glfwGetVideoMode(m_monitor) : nullptr;
		const int fullscreenWidth = mode ? mode->width : static_cast<int>(m_windowData.m_CurrentWidth);
		const int fullscreenHeight = mode ? mode->height : static_cast<int>(m_windowData.m_CurrentHeight);
		if (mode)
		{
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		}

		m_window = glfwCreateWindow(fullscreenWidth,
			fullscreenHeight,
			winTitle.c_str(),
			m_monitor, nullptr);

		m_windowData.m_WindowWidth = static_cast<unsigned int>(fullscreenWidth);
		m_windowData.m_WindowHeight = static_cast<unsigned int>(fullscreenHeight);
#else
		m_window = glfwCreateWindow(static_cast<int>(m_windowData.m_CurrentWidth),
			static_cast<int>(m_windowData.m_CurrentHeight),
			winTitle.c_str(),
			nullptr, nullptr);

		m_windowData.m_WindowWidth = m_windowData.m_CurrentWidth;
		m_windowData.m_WindowHeight = m_windowData.m_CurrentHeight;
#endif

		if (!m_window)
		{
			std::cout << "Failed to create window !" << std::endl;
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_windowData);

		NOCTURNAL_ENGINE_INFO("Generating Window {0} : SIZE ({1},{2})", m_windowData.Title, m_windowData.m_Width, m_windowData.m_Height);
		

		if(glewInit() != GLEW_OK)
			NOCTURNAL_ENGINE_ASSERT(!glewInit(), "GLEW init has failed - abort program...");

		//context
		ToggleVsync(true);

		glfwSetCursorPosCallback(m_window, Mouseposition_callback);
		glfwSetKeyCallback(m_window, Key_callback);
		glfwSetScrollCallback(m_window, Mousescroll_callback);
		glfwSetMouseButtonCallback(m_window, Mousebutton_callback);
		glfwSetFramebufferSizeCallback(m_window, Window_size_callback);
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetDropCallback(m_window, drop_callback);
#if !DEBUG
		glfwSetWindowFocusCallback(m_window, OnWindowFocus);
#endif

		int framebufferWidth = 0, framebufferHeight = 0;
		glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
		Window_size_callback(m_window, framebufferWidth, framebufferHeight);

		/* Make the window's context current */
	}


	void Window::Show() {
#if 0
#if !DEBUG		
		const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);
		m_windowData.m_Width = mode->width;
		m_windowData.m_Height = mode->height;

		glfwSetWindowMonitor(
			m_window, m_monitor,
			0, 0,
			m_windowData.m_Width, m_windowData.m_Height, 0);
#endif	
#endif
		// Finally show the window
		glfwShowWindow(m_window);
	}
	/*!*************************************************************************
	Update loop of window
	****************************************************************************/
	void Window::Update(float frametime)
	{
		(void)frametime;	
		/* Poll for and process events */
		glfwPollEvents();
		/* Swap front and back buffers */
		glfwSwapBuffers(m_window);

	}
  
	/*!*************************************************************************
	End loop of window
	****************************************************************************/
	void Window::End()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	/*!*************************************************************************
	drop_callback function is the function definition for the glfwSetDropCallback
	function parameter to enable drag and drop functionality to drag and drop
	files from external folders to the game engine.  
	****************************************************************************/
	void Window::drop_callback(GLFWwindow* window, int count, const char** paths)
	{
		UNREFERENCED_PARAMETER(window);
		for (int i = 0; i < count; i++)
		{
			if (std::filesystem::path(paths[i]).extension() == ".wav")
			{
				//copy the wav file to Audio folder
				auto folder = std::filesystem::path("Assets/Audio");
				auto filename = std::filesystem::path(paths[i]).filename();
				std::filesystem::copy(std::filesystem::path(paths[i]), folder / filename);
				//insert entry into audio file paths
				p_Editor->insertAudioFilePath(paths[i]);
			}

			else if (std::filesystem::path(paths[i]).extension() == ".png")
			{
				auto filename = std::filesystem::path(paths[i]).filename();
				std::filesystem::path folder = {};
				if (filename.string().at(0) == 'C')
				{
					folder = "Assets/Textures/Characters";
				}
				else if (filename.string().at(0) == 'E')
				{
					folder = "Assets/Textures/Environment";
				}
				else if (filename.string().at(0) == 'U')
				{
					folder = "Assets/Textures/UI";
				}
				else if (filename.string().at(0) == 'V')
				{
					folder = "Assets/Textures/VFX";
				}
				else NOCTURNAL_ENGINE_INFO("The name convention for this png {0} is wrong", filename.string().c_str());

				if (!std::filesystem::exists(folder)||!std::filesystem::is_directory(folder))
				{
					std::filesystem::create_directory(folder);
				}
				std::filesystem::copy(std::filesystem::path(paths[i]), folder / filename);
			}
			else
			{
				NOCTURNAL_ENGINE_INFO("Error Detected The extenstion is {0}", std::filesystem::path(paths[i]).extension().string().c_str());
			}
		}
	}

	/*!*************************************************************************
	spdlog assert for window error
	****************************************************************************/
	void Window::ErrorCallback(int error, const char* description)
	{
		NOCTURNAL_ENGINE_ERROR("GLFW ERROR {0} : {1}", error, description);
	}

	/*!*************************************************************************
	Window size callback function
	****************************************************************************/
	void Window::Window_size_callback(GLFWwindow* window, int width, int height)
	{
		WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);
		if (width <= 0 || height <= 0)
		{
			return;
		}

		data.m_WindowWidth = static_cast<unsigned int>(width);
		data.m_WindowHeight = static_cast<unsigned int>(height);

#if !DEBUG
		const unsigned int virtualWidth = data.m_CurrentWidth > 0 ? data.m_CurrentWidth : static_cast<unsigned int>(width);
		const unsigned int virtualHeight = data.m_CurrentHeight > 0 ? data.m_CurrentHeight : static_cast<unsigned int>(height);

		const float scaleX = static_cast<float>(width) / static_cast<float>(virtualWidth);
		const float scaleY = static_cast<float>(height) / static_cast<float>(virtualHeight);
		const float scale = (std::min)(scaleX, scaleY);

		const int viewportWidth = (std::max)(1, static_cast<int>(virtualWidth * scale));
		const int viewportHeight = (std::max)(1, static_cast<int>(virtualHeight * scale));
		const int viewportX = (width - viewportWidth) / 2;
		const int viewportY = (height - viewportHeight) / 2;

		data.m_Width = static_cast<unsigned int>(viewportWidth);
		data.m_Height = static_cast<unsigned int>(viewportHeight);
		data.m_ViewportX = static_cast<unsigned int>(viewportX);
		data.m_ViewportY = static_cast<unsigned int>(viewportY);

		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
#else
		data.m_Width = static_cast<unsigned int>(width);
		data.m_Height = static_cast<unsigned int>(height);
		data.m_ViewportX = 0;
		data.m_ViewportY = 0;

		glViewport(0, 0, width, height);
#endif
	}

	/*!*************************************************************************
	Window key callback function
	****************************************************************************/
	void Window::Key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		UNREFERENCED_PARAMETER(window);
		(void)scancode, (void)mode;
		InputSystem::GetInstance()->SetKeyStatus(key, action);		
	}

	/*!*************************************************************************
	Window mouse button callback function
	****************************************************************************/
	void Window::Mousebutton_callback(GLFWwindow* window, int button, int action, int mode)
	{
		(void)window, (void)mode;
		InputSystem::GetInstance()->SetMouseStatus(button, action);
	}

	/*!*************************************************************************
	Window mousescroll callback function
	****************************************************************************/
	void Window::Mousescroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		(void)window, (void)xoffset, (void)yoffset;
		InputSystem::GetInstance()->mMouseScrollStatus = static_cast<int>(yoffset);
	}

	/*!*************************************************************************
	Window mouse position callback function
	****************************************************************************/
	void Window::Mouseposition_callback(GLFWwindow* window, double xpos, double ypos)
	{
		
		WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);
		data.mouseX = xpos;
		data.mouseY = ypos;

		p_Input->SetLastDevice(InputDevice::KeyboardMouse);
	}


	/*!*************************************************************************
	Toggle vsync function
	****************************************************************************/
	void Window::ToggleVsync(bool value)
	{
		if (value)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_windowData.Vsync = value;
	}

	void Window::OnWindowFocus(GLFWwindow* window, int focused)
	{
		if (!focused)
		{
			// Window lost focus (Alt-Tab or similar)
			int width, height;
			glfwGetWindowSize(window, &width, &height);

			// Minimize the window or hide it
			// depending on your game's behavior
			glfwIconifyWindow(window);

			if (!isWindowNotFocus)
			{
				isWindowNotFocus = true;
				for (auto i = p_Audio->mChannelMap.begin(); i != p_Audio->mChannelMap.end(); i++)
				{
					p_Audio->PauseSound(i->first);
				}
			}
		}
		else
		{
			glfwRestoreWindow(window);
			if (isWindowNotFocus)
			{
				isWindowNotFocus = false;
				for (auto i = p_Audio->mChannelMap.begin(); i != p_Audio->mChannelMap.end(); i++)
				{
					p_Audio->UnpauseSound(i->first);					
				}
			}
		}
	}

}