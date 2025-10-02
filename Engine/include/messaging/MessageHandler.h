#pragma once

#include "Message.h"

namespace Messaging
{
    class MessageHandler
    {
    public:
        virtual ~MessageHandler() = default;
        virtual void HandleMessage(const Message& msg) = 0;
    };
}