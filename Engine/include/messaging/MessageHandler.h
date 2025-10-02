/*!------------------------------------------------------------------------------------------------
    File: MessageHandler.h
    Project: NocturnalStudio

    This file contains the messaging handler

    Primary Author:
        Nur Syazliyarni (100%)
        - Created...

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Message.h"

namespace Messaging
{
    class IMessageHandler
    {
    public:
        virtual ~IMessageHandler() = default;
        virtual void HandleMessage(Message* msg) = 0;
    };
}