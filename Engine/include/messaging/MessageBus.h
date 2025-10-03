/*!------------------------------------------------------------------------------------------------
    File: MessageBus.h
    Project: NocturnalStudio

    This file contains the functions register, deregister and broadcast

    Primary Author:
        Nur Syazliyarni (100%)
        - Created for the messaging system using a basic broadcasting system where 
         we can register specific listeners to get the messages

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include "MessageHandler.h"

namespace Messaging
{
    //broadcast hub for all messages
    class MessageBus
    {
    public:
        void Register(IMessageHandler* handler) {
            handlers.push_back(handler);
        }

        void Unregister(IMessageHandler* handler) {
            handlers.erase(
                std::remove(handlers.begin(), handlers.end(), handler),
                handlers.end()
            );
        }

        //sends a message to registered handlers
        void Broadcast(std::unique_ptr<Message> msg) {
            for (auto* h : handlers) {
                h->HandleMessage(msg.get());  // pass raw pointer for read-only use
            }
            // msg automatically destroyed here when unique_ptr goes out of scope
        }

    private:
        std::vector<IMessageHandler*> handlers;
    };
}