/*!------------------------------------------------------------------------------------------------
    File: RevealTargetOnClick.h
    Project: NocturnalStudio

    Brief:
        Reveals a target entity when this entity is clicked.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created the RevealTargetOnClick script and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IScript.h"
#include "Input/Input.h"
#include "Math/Vmath.h"

namespace NocturnalEngine
{
    class RevealTargetOnClick : public IScript
    {
    public:
        RevealTargetOnClick();
        ~RevealTargetOnClick() = default;

        virtual RevealTargetOnClick* Clone() const override { return new RevealTargetOnClick(*this); }
        virtual void Start() override;
        virtual void Update(float Frametime) override;
        virtual void End() override {}
        virtual std::string GetScriptName() override { return "RevealTargetOnClick"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        std::unique_ptr<InputSystem>& mInput;
        Entity mTarget{ 0 };
        vec2D mTargetScale{ 0.0f, 0.0f };
        bool mHasTarget{ false };
        bool mVisible{ false };

        void FindTargetEntity();
        void HideTarget();
        void ToggleTarget();
        bool IsMouseOver(Entity entity) const;
    };
}
