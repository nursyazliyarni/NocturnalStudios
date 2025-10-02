#pragma once

#include "messaging/Message.h"

namespace InputMessaging {

	class KeyMessage : public Messaging::Message {
		public:
        KeyMessage(int key, bool pressed)
            : Message(Messaging::Mid::KEY_EVENT), keyCode(key), isPressed(pressed) {
        }

        int keyCode;
        bool isPressed;
    };

    // Mouse message
    class MouseMessage : public Messaging::Message {
    public:
        MouseMessage(int button, int x, int y)
            : Message(Messaging::Mid::MOUSEBUTTON), button(button), x(x), y(y) {
        }

        int button;
        int x, y;
    };
	
}