/*!------------------------------------------------------------------------------------------------
    File: InputMessage.h
    Project: NocturnalStudio

    This file contains the messaging classes for the input system

    Primary Author:
        Nur Syazliyarni (100%)
        - Created...

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "messaging/Message.h"

namespace Messaging {

    //Message class for when a key is pressed
	class KeyMessage : public Message {
		public:
        KeyMessage(int key, int action)
            : Message(Mid::CHARACTERKEY), keyCode(key), action(action) {
        }

        int keyCode; // 
        int action; // key released or pressed
    };

    // Message class for when the mouse is clicked
    /*class MovingMouseMessage : public Message {
    public:
        MovingMouseMessage( float x, float y)
            : Message(Mid::MOUSEMOVE), x(x), y(y) {
        }

        float x, y;
    };*/

    //message class for when mouse is clicked
    class ClickedMouseMessage : public Message {
    public:
        ClickedMouseMessage(int button, bool pressed, float x, float y)
            : Message(Mid::MOUSEBUTTON), MouseButtonIndex(button),
            buttonPressed(pressed), mouseX(x), mouseY(y) {
        }


        int MouseButtonIndex; //mouse button left or right
        bool buttonPressed; 
        float mouseX; //mouse pos x
        float mouseY; //mouse pos  y
    };
	
}