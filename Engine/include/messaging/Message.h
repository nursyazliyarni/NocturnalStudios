/*!------------------------------------------------------------------------------------------------
	File: Message.h
	Project: NocturnalStudio

	This file contains the enum message ids and base class message

	Primary Author:
		Nur Syazliyarni (100%)
		- Created...

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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