#pragma once

#include "Scripts/IScript.h"

namespace NocturnalEngine
{
    // Updates a Text component to show how many encryption keys were collected.
    class KeysCollectedDisplay : public IScript
    {
    public:
        KeysCollectedDisplay() = default;
        ~KeysCollectedDisplay() = default;

        virtual KeysCollectedDisplay* Clone() const override
        {
            return new KeysCollectedDisplay(*this);
        }

        virtual void Start() override;
        virtual void Update(float Frametime) override;
        virtual void End() override {}

        bool Requirements(Entity e) override {
            return p_ecs.HaveComponent<Text>(e);
        }

        virtual std::string GetScriptName() override { return "KeysCollectedDisplay"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        void RefreshText() const;
    };
}
