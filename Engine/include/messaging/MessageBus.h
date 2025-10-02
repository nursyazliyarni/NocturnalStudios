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