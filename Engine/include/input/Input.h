/*!------------------------------------------------------------------------------------------------
    File: Input.h
    Project: NocturnalStudio

	Brief:
		This file contain the main function for input system
		and the functions of the keyboard and mouse.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Input.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Core/Core.h"

#define p_Input NocturnalEngine::InputSystem::GetInstance()

namespace NocturnalEngine {

	template class NOCTURNAL_ENGINE_API std::vector <int, std::allocator<int>>;
	template class NOCTURNAL_ENGINE_API std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, std::allocator<std::pair<const int, int>>>;

	using keystatus = int;
	using keycode = int;
	using mousestatus = int;
	using mousecode = int;

	class NOCTURNAL_ENGINE_API InputSystem
	{
	public:
		//Pointer to input system
		static std::unique_ptr<InputSystem>& GetInstance();

		// key inputs 
		bool KeyPressed(const int& key);
		bool KeyHold(const int& key);
		bool KeyReleased(const int& key);

		void SetKeyStatus(keycode key, keystatus status);
		void ResetPressedKey();

		//mouse inputs
		bool MousePressed(const int& key);
		bool MouseHold(const int& key);
		bool MouseIsReleased(const int& key);

		void SetMouseStatus(mousecode key, mousestatus status);
		void ResetPressedMouse();

		bool isKeyPressed(keycode key);


        // Gamepad
        // Call once per frame (after glfwPollEvents) to sync gamepad state.
        void PollGamepad(int joystickSlot = 0);  // slot 0 = player 1

        bool IsGamepadConnected()const;
        bool GamepadPressed(GamepadButton btn)const; // true on the frame it's first pressed
        bool GamepadHeld(GamepadButton btn)const; // true while held
        bool GamepadReleased(GamepadButton btn)const; // true on the frame it's released
        float GetAxis(GamepadAxis axis)const; // -1.0�1.0 (triggers: 0.0�1.0)

		bool GamepadGotDisconnected(){
			if (controllerWasDisconnected) {
				controllerWasDisconnected = false;
				return true;
			}
			else return controllerWasDisconnected;
		}

		bool GamepadGotConnected() {
			if (coltrollerWasConnected) {
				coltrollerWasConnected = false;
				return true;
			}
			else return coltrollerWasConnected;
		}

        void ResetGamepadButtons(); // call alongside ResetPressedKey each frame

		InputDevice GetLastDevice() const { return mLastDevice; }
		void SetLastDevice(InputDevice id) { 
			if (mLastDevice!=id)deviceChanged = true;
			mLastDevice = id; 
		}

		bool DeviceChanged() {
			if (deviceChanged)
			{
				deviceChanged = false;
				return true;
			}
			else return deviceChanged;
		}

	public:
		//data
		std::unordered_map<keycode, keystatus> mKeyStatus;
		std::unordered_map<mousecode, mousestatus> mMouseStatus;
		std::vector<keycode> mReset;
		std::vector<mousecode> mouseReset;

		double mMouseScrollStatus = 0.0f;

        bool  mGamepadConnected = false,wasGamepadConnected=false;

		bool coltrollerWasConnected{ false }, controllerWasDisconnected{ false };

        unsigned char mGamepadCurr[15] = {};
        unsigned char mGamepadPrev[15] = {};

        float mGamepadAxes[6] = {};

        static float ApplyDeadzone(float val)
        {
            return (std::abs(val) < DEADZONE) ? 0.0f : val;
        }

		InputDevice mLastDevice = InputDevice::KeyboardMouse;
		bool deviceChanged{ false };
	};
}
