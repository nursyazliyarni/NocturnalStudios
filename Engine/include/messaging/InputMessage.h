#pragma once

#include "messaging/Message.h"

namespace Messaging {

    //Message class for when a key is pressed
	class KeyMessage : public Message {
		public:
        KeyMessage(int key, int action)
            : Message(Mid::KEY_EVENT), keyCode(key), action(action) {
        }

        int keyCode;
        int action;
    };

    // Message class for when the mouse is clicked
    /*class MovingMouseMessage : public Message {
    public:
        MovingMouseMessage( float x, float y)
            : Message(Mid::MOUSEMOVE), x(x), y(y) {
        }

        float x, y;
    };*/

    class ClickedMouseMessage : public Message {
    public:
        ClickedMouseMessage(int button, bool pressed, float x, float y)
            : Message(Mid::MOUSEBUTTON), MouseButtonIndex(button),
            buttonPressed(pressed), mouseX(x), mouseY(y) {
        }


        int MouseButtonIndex;
        bool buttonPressed;
        float mouseX;
        float mouseY;
    };
	
}