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

	//base class message with only message id
	class Message
	{
	public:
		explicit Message(Mid::MessageIdType id) : MessageId(id) {} 
		virtual ~Message() = default;

		Mid::MessageIdType MessageId;
	};

}