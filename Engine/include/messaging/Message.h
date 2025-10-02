#pragma once
#include "../math/Vector.h"
#include <cstdint>

namespace Messaging
{
	namespace Mid // Message id
	{
		enum MessageIdType
		{
			NONE,
			QUIT,
			COLLIDE,
			CHARACTERKEY,
			MOUSEBUTTON,
			MOUSEMOVE,
			KEY_EVENT,
			MOUSE_EVENT
		};
	}

	///Base message class. New message types are defined by deriving from this
	///class and mapping defined an element of the MessageIdType enumeration.
	class Message
	{
	public:
		Message(Mid::MessageIdType id) : MessageId(id) {};
		Mid::MessageIdType MessageId;
		virtual ~Message() {};
	};

	/*
	//message for pressed key
	class MessageCharacterKey : public Message
	{
	public:
		MessageCharacterKey() : Message(Mid::CHARACTERKEY) {};
		int character;
	};

	//detect when has been clickedd
	class MouseButton : public Message
	{
	public:
		enum MouseButtonIndexId
		{
			LeftMouse,
			RightMouse
		};

		MouseButton(MouseButtonIndexId button, bool state, Vector2D position)
			: Message(Mid::MOUSEBUTTON), MouseButtonIndex(button), ButtonIsPressed(state), MousePosition(position) {
		};

		MouseButtonIndexId MouseButtonIndex;
		bool ButtonIsPressed;
		Vector2D MousePosition;
	};
	*/
}