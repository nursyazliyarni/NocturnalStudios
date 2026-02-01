/*!------------------------------------------------------------------------------------------------
    File: RevealTargetOnClick.cpp
    Project: NocturnalStudio

    Brief:
        Reveals a target entity when this entity is clicked.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created the RevealTargetOnClick script and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/RevealTargetOnClick.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "GUI/GUI.h"
#include "Math/Vmath.h"

namespace NocturnalEngine
{
    RevealTargetOnClick::RevealTargetOnClick()
        : mInput{ InputSystem::GetInstance() }
    {
    }

    void RevealTargetOnClick::Start()
    {
        FindTargetEntity();
        HideTarget();
    }

    void RevealTargetOnClick::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (!mHasTarget) // Skip if we didn't find the target entity
            return;

        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && IsMouseOver(entityID)) // Click on OwlMedicine to toggle
        {
            ToggleTarget();
        }

        if (mVisible && p_ecs.HaveComponent<Transform>(mTarget)) // Follow cursor when visible
        {
            auto& transform = p_ecs.GetComponent<Transform>(mTarget);
            transform.SetPos(p_GUI->MousePosition.x, p_GUI->MousePosition.y);
        }
    }

    void RevealTargetOnClick::FindTargetEntity()
    {
        for (Entity e : p_ecs.GetAliveEntitiesSet()) // Search for the entity named "target"
        {
            if (!p_ecs.HaveComponent<NameTag>(e))
                continue;

            if (p_ecs.GetComponent<NameTag>(e).GetNameTag() == "target")
            {
                mTarget = e;
                mHasTarget = true;
                break;
            }
        }
    }

    void RevealTargetOnClick::HideTarget()
    {
        if (!mHasTarget || !p_ecs.HaveComponent<Transform>(mTarget)) // Ensure target has a Transform
            return;

        auto& transform = p_ecs.GetComponent<Transform>(mTarget);
        mTargetScale = transform.GetScale();
        transform.SetScale(0.0f, 0.0f); // Hide target on start
        mVisible = false;
    }

    void RevealTargetOnClick::ToggleTarget()
    {
        if (!mHasTarget || !p_ecs.HaveComponent<Transform>(mTarget)) // Ensure target is valid
            return;

        auto& transform = p_ecs.GetComponent<Transform>(mTarget);
        if (mVisible) // Hide when currently visible
        {
            transform.SetScale(0.0f, 0.0f);
            mVisible = false;
        }
        else
        {
            // Restore original scale when showing
            transform.SetScale(mTargetScale.x, mTargetScale.y);
            mVisible = true;
        }
    }

    bool RevealTargetOnClick::IsMouseOver(Entity entity) const
    {
        if (!p_ecs.HaveComponent<Transform>(entity) || !p_ecs.HaveComponent<Collider>(entity)) // Need transform + collider
            return false;

        vec2D mousePos{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        auto& transform = p_ecs.GetComponent<Transform>(entity);
        auto& collider = p_ecs.GetComponent<Collider>(entity);

        for (int i = 0; i < 2; ++i) // Check both collider slots
        {
            vec2D offsetPos{ transform.GetPos().x + collider[i].mOffset.x,
                             transform.GetPos().y + collider[i].mOffset.y };

            switch (collider[i].mCol) // Handle circle and box hit tests
            {
            case Collider::ColliderType::circle:
            case Collider::ColliderType::bubble:
            case Collider::ColliderType::bossball:
                if (distance(mousePos, offsetPos) < collider[i].mRadius)
                    return true;
                break;
            case Collider::ColliderType::rect:
            case Collider::ColliderType::box:
            {
                vec2D minPt{ offsetPos.x - collider[i].mMin.x, offsetPos.y - collider[i].mMin.y };
                vec2D maxPt{ offsetPos.x + collider[i].mMax.x, offsetPos.y + collider[i].mMax.y };
                if (mousePos.x > minPt.x && mousePos.x < maxPt.x &&
                    mousePos.y > minPt.y && mousePos.y < maxPt.y)
                {
                    return true;
                }
                break;
            }
            default:
                break;
            }
        }

        return false;
    }
}
