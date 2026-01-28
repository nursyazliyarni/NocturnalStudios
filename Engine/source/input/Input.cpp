/*!------------------------------------------------------------------------------------------------
    File: Input.cpp
    Project: NocturnalStudio

	Brief:
		This file contain the input manager which is responsible for retrieving keyboard and mouse input.
        The input systems provides us with a way to fetch information on inputs in a simple
        and effective manner. Allowing the user to view whether the keys or mouse are pressed,
        released or being held.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the InputSystem class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Input/Input.h"
#include "Window/Window.h"
#include "GUI/GUI.h"
#include <GLFW/glfw3.h>

namespace NocturnalEngine {

    std::unique_ptr<InputSystem> mInstance;
    std::unique_ptr<InputSystem>& InputSystem::GetInstance()
    {
        if (!mInstance)
        {
            mInstance = std::make_unique<InputSystem>();
        }
        return mInstance;
    }

    /*!*************************************************************************
    Store key pressed input feedback in vector mReset
    ****************************************************************************/
    bool InputSystem::KeyPressed(const int& key)
    {
        return mKeyStatus[key] == GLFW_PRESS;
    }

    /*!*************************************************************************
    Store key hold input feedback status in mKeyStatus
    ****************************************************************************/
    bool InputSystem::KeyHold(const int& key)
    {
        return mKeyStatus[key] == GLFW_REPEAT || mKeyStatus[key] == GLFW_PRESS;
    }

    /*!*************************************************************************
    Store key released input feedback in both vector mReset and mKeyStatus
    ****************************************************************************/
    bool InputSystem::KeyReleased(const int& key)
    {
        return mKeyStatus[key] == GLFW_RELEASE;
    }

    /*!*************************************************************************
    Set keystatus based on keypressed
    ****************************************************************************/
    void InputSystem::SetKeyStatus(keycode key, keystatus status)
    {
        mReset.emplace_back(key);
        mKeyStatus[key] = status;
    }

    /*!*************************************************************************
    Resets mReset vector and mKeyStatus map to empty
    ****************************************************************************/
    void InputSystem::ResetPressedKey()
    {
        for (auto& key : mReset)
        {
            if (mKeyStatus[key] == GLFW_PRESS)
                mKeyStatus[key] = GLFW_REPEAT;
        }

        mReset.clear();
    }

    /*!*************************************************************************
    Store mouse input feedback if mouse is pressed
    ****************************************************************************/
    bool InputSystem::MousePressed(const int& key)
    {
        return mMouseStatus[key] == GLFW_PRESS;
    }

    /*!*************************************************************************
    Store mouse input feedback if mouse is held
    ****************************************************************************/
    bool InputSystem::MouseHold(const int& key)
    {
        return  mMouseStatus[key] == GLFW_REPEAT;
    }

    /*!*************************************************************************
    Store mouse input feedback if mouse is released
    ****************************************************************************/
    bool InputSystem::MouseIsReleased(const int& key)
    {
        return mMouseStatus[key] == GLFW_RELEASE;
    }

    /*!*************************************************************************
    Set mouse status
    ****************************************************************************/
    void InputSystem::SetMouseStatus(mousecode key, mousestatus status)
    {
        mouseReset.emplace_back(key);
        mMouseStatus[key] = status;
    }

    /*!*************************************************************************
    Reset mouse input status in mMouseStatus to 0
    ****************************************************************************/
    void InputSystem::ResetPressedMouse()
    {
        for (auto& key : mouseReset)
        {
            if (mMouseStatus[key] == GLFW_PRESS)
                mMouseStatus[key] = GLFW_REPEAT;
        }

        mouseReset.clear();
    }

    /*!*************************************************************************
    Checks if key is pressed by finding mKeyStatus
    ****************************************************************************/
    bool InputSystem::isKeyPressed(keycode key)
    {
        auto it = mKeyStatus.find(key);
        if (it != mKeyStatus.end())
            return it->second == GLFW_PRESS; //find the key in the map and execute the order

        return false;

    }

    void InputSystem::detect_draggable_item(mousecode mouse, bool is_editormode)
    {
        static bool is_mouse_dragging{ false };
        static vec2D drag_offset{}; static Entity animal_entity{};
        if (!is_editormode) {
            for (Entity i{ 0 }; i < p_ecs.GetTotalEntities(); ++i) {
                if (p_ecs.HaveComponent<AnimalAttributes>(i)) {
                    animal_entity = i;
                }

                if (p_ecs.HaveComponent<ItemsAttributes>(i)) {
                    auto& transform_data{ p_ecs.GetComponent<Transform>(i) };
                    if (MousePressed(mouse)&&!is_mouse_dragging) {
                        if (p_GUI->MousePosition.x >= transform_data.GetPos().x - transform_data.GetScale().x / 2 &&
                            p_GUI->MousePosition.x <= transform_data.GetPos().x + transform_data.GetScale().x / 2 &&
                            p_GUI->MousePosition.y >= transform_data.GetPos().y - transform_data.GetScale().y / 2 &&
                            p_GUI->MousePosition.y <= transform_data.GetPos().y + transform_data.GetScale().y / 2)
                        {
                            is_mouse_dragging = true;
                            p_ecs.GetComponent<ItemsAttributes>(i).is_dragging = true;
                            p_ecs.GetComponent<ItemsAttributes>(i).duplicate_item_pos = { transform_data.GetPos().x,transform_data.GetPos().y };
                            drag_offset = { p_ecs.GetComponent<ItemsAttributes>(i).duplicate_item_pos.x - p_GUI->MousePosition.x,
                            p_ecs.GetComponent<ItemsAttributes>(i).duplicate_item_pos.y - p_GUI->MousePosition.y };
                        }
                    }

                    else if (MouseIsReleased(mouse)) {
                        if (p_ecs.GetComponent<ItemsAttributes>(i).is_dragging) {
                            is_mouse_dragging = false;
                            p_ecs.GetComponent<ItemsAttributes>(i).is_dragging = false;
                        }
                    }

                    if (p_ecs.GetComponent<ItemsAttributes>(i).is_dragging) {
                        auto& duplicate_item_pos{ p_ecs.GetComponent<ItemsAttributes>(i).duplicate_item_pos };
                        duplicate_item_pos = { p_GUI->MousePosition.x + drag_offset.x,
                            p_GUI->MousePosition.y + drag_offset.y };

                        auto& animal_transform{ p_ecs.GetComponent<Transform>(animal_entity) };
                        if (duplicate_item_pos.x+transform_data.GetScale().x/2>=animal_transform.GetPos().x - animal_transform.GetScale().x / 2 &&
                            duplicate_item_pos.x-transform_data.GetScale().x/2<=animal_transform.GetPos().x + animal_transform.GetScale().x / 2 &&
                            duplicate_item_pos.y+transform_data.GetScale().y/2>=animal_transform.GetPos().y - animal_transform.GetScale().y / 2 &&
                            duplicate_item_pos.y-transform_data.GetScale().y/2<=animal_transform.GetPos().y + animal_transform.GetScale().y / 2)
                        {
                            is_mouse_dragging = false;
                            p_ecs.GetComponent<ItemsAttributes>(i).is_dragging = false;
                        }
                    }

                }
            }
        }

    }

}