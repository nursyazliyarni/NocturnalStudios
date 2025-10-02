#pragma once
#include <vector>
#include <algorithm>
#include "MessageHandler.hpp"

namespace Messaging
{
    class MessageBus
    {
    public:
        void Register(MessageHandler* handler) {
            handlers.push_back(handler);
        }

        void Unregister(MessageHandler* handler) {
            handlers.erase(
                std::remove(handlers.begin(), handlers.end(), handler),
                handlers.end()
            );
        }

        void Broadcast(const Message& msg) {
            for (auto* h : handlers) {
                h->HandleMessage(msg);
            }
        }

    private:
        std::vector<IMessageHandler*> handlers;
    };
}