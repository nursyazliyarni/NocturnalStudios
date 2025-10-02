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