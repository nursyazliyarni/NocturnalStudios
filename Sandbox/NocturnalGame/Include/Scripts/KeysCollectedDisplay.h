/*!------------------------------------------------------------------------------------------------
    File: KeysCollectedDisplay.h
    Project: NocturnalStudios

    Date:
        2026-04-05

    Brief:
        Declares the script that updates the collected-keys counter text.

    Primary Author:
         Nur Syazliyarni (100%)
        - Added the file header.
        - Email: nursyazliyarni.b

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
