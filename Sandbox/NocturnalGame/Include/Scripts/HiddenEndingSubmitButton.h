#pragma once

#include "Scripts/IScript.h"
#include "Input/Input.h"
#include "Math/Vmath.h"

namespace NocturnalEngine
{
    class HiddenEndingSubmitButton : public IScript
    {
    public:
        HiddenEndingSubmitButton();
        ~HiddenEndingSubmitButton() = default;

        virtual HiddenEndingSubmitButton* Clone() const override
        {
            return new HiddenEndingSubmitButton(*this);
        }

        virtual void Start() override {}
        virtual void Update(float Frametime) override;
        virtual void End() override {}

        bool Requirements(Entity e) override
        {
            return p_ecs.HaveComponent<Collider>(e) && p_ecs.HaveComponent<Transform>(e);
        }

        virtual std::string GetScriptName() override { return "HiddenEndingSubmitButton"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        bool IsMouseOver(Entity entity) const;

        std::unique_ptr<InputSystem>& mInput;
        bool mPressedInside{ false };
    };
}
