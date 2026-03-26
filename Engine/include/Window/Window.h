/*!------------------------------------------------------------------------------------------------
    File: Window.h
    Project: NocturnalStudio

	Brief:
		This file contains the Window class declaration which provides the implementation
		for creating and managing a window using GLFW and OpenGL. It includes functionality
		for handling window properties, input events, and integrating with ImGui for GUI rendering.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Window.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Engine.h"
#include "Core/Core.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ECS/Components/Components.h"
#include <windows.h>


namespace NocturnalEngine {
	class NOCTURNAL_ENGINE_API WinData;
	//properties used in window
	struct WindowProps 
	{
		std::string Title;
		unsigned int m_Width,
					 m_Height,
					 m_CurrentWidth,
					 m_CurrentHeight;
		bool Vsync;
		double mouseX, mouseY;
	};

	class NOCTURNAL_ENGINE_API Window : public System
	{
	public:
		Window();

		~Window() = default;

	public:
		
		//Initialize
		virtual void Init() override;

		//Show- to be displayed after all other systems needed are ready
		void Show();
		//update
		virtual void Update(float frametime) override;

		//Delete window
		virtual void End() override;

		//Getter
		//Window name
		virtual std::string GetName() override { return m_windowData.Title; }
		WindowProps Getter() const { return m_windowData; }
		GLFWwindow* GetWindow() const { return m_window; }
		GLFWmonitor* GetMonitor() const { return m_monitor; }
		
		void HideShowCursor();

	public:

		static bool isWindowNotFocus;

	private:
		GLFWwindow* m_window;
		GLFWmonitor* m_monitor;
		WinData windowData;
		WindowProps m_windowData;
		bool m_vsync;
		double previousTime;
		int frameCount;

	private:
		// Event callback functions
		static void ErrorCallback(int error, const char* description); // error
		static void Window_size_callback(GLFWwindow* window, int width, int height); // whenever window size changes, this function is called
		static void Key_callback(GLFWwindow* window, int key, int scancode, int action, int mode); // key input
		static void Mousebutton_callback(GLFWwindow* window, int button, int action, int mode); // mouse input
		static void Mousescroll_callback(GLFWwindow* window, double xoffset, double yoffset); // mouse scrolling
		static void Mouseposition_callback(GLFWwindow* window, double xpos, double ypos); // mouse position
		static void drop_callback(GLFWwindow* window, int count, const char** paths);
		void ToggleVsync(bool value);// vync based on internal
		static void OnWindowFocus(GLFWwindow* window, int focused);
	};
}