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
        // Finds required scene entities and hides the target on scene start.
        virtual void Start() override;
        // Handles click input, target toggling, and pain spot consume/heal logic.
        virtual void Update(float Frametime) override;
        virtual void End() override {}
        virtual std::string GetScriptName() override { return "RevealTargetOnClick"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        std::unique_ptr<InputSystem>& mInput;
        Entity mTarget{ 0 };
        Entity mOwl{ 0 };
        vec2D mTargetScale{ 0.0f, 0.0f };
        bool mHasTarget{ false };
        bool mVisible{ false };

        // Locates the reveal icon target entity using Tag == "target".
        void FindTargetEntity();
        // Locates the owl entity used for health updates.
        void FindOwlEntity();
        // Stores original target scale and hides target.
        void HideTarget();
        // Shows or hides the target sprite.
        void ToggleTarget();
        // Removes one pain spot under target center and heals owl by 15%.
        void ConsumePainSpotAndHeal();
        // Returns true if target center is inside any pain spot.
        bool IsTargetOverPainSpot(Entity& outSpot) const;
        // Mouse-over test for the script owner collider (medicine icon).
        bool IsMouseOver(Entity entity) const;
    };
}
