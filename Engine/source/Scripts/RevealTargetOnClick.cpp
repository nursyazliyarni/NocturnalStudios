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
#include "ECS/Components/AnimalAttributes.h"
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
        FindOwlEntity();
        HideTarget();
    }

    void RevealTargetOnClick::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (!mHasTarget) // Skip if we didn't find the target entity
            return;

        if (mVisible && p_ecs.HaveComponent<Transform>(mTarget)) // Follow cursor when visible
        {
            auto& transform = p_ecs.GetComponent<Transform>(mTarget);
            transform.SetPos(p_GUI->MousePosition.x, p_GUI->MousePosition.y);
        }

        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && IsMouseOver(entityID)) // Click on OwlMedicine to toggle
        {
            ToggleTarget();
            return;
        }

        if (mVisible && mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            ConsumePainSpotAndHeal();
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

    void RevealTargetOnClick::FindOwlEntity()
    {
        mOwl = 0;
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<AnimalAttributes>(e) || !p_ecs.HaveComponent<NameTag>(e))
                continue;

            if (p_ecs.GetComponent<NameTag>(e).GetNameTag() == "Owl")
            {
                mOwl = e;
                return;
            }
        }

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (p_ecs.HaveComponent<AnimalAttributes>(e))
            {
                mOwl = e;
                return;
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

    void RevealTargetOnClick::ConsumePainSpotAndHeal()
    {
        Entity painSpot = 0;
        if (!IsTargetOverPainSpot(painSpot))
            return;

        p_ecs.DestroyEntity(painSpot);

        if (mOwl == 0 || !p_ecs.HaveComponent<AnimalAttributes>(mOwl))
            return;

        auto& attrs = p_ecs.GetComponent<AnimalAttributes>(mOwl);
        const double maxHealth = attrs.maxHealth > 0.0 ? attrs.maxHealth : 1.0;
        const double healAmount = maxHealth * 0.15;
        attrs.current_health = std::min(maxHealth, attrs.current_health + healAmount);
    }

    bool RevealTargetOnClick::IsTargetOverPainSpot(Entity& outSpot) const
    {
        outSpot = 0;

        if (!mVisible || !mHasTarget || !p_ecs.HaveComponent<Transform>(mTarget))
            return false;

        auto& targetTransform = p_ecs.GetComponent<Transform>(mTarget);
        const vec2D targetPos = targetTransform.GetPos();
        const vec2D targetScale = targetTransform.GetScale();
        const float targetRadius = std::max(std::abs(targetScale.x), std::abs(targetScale.y)) * 0.5f;

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (e == mTarget)
                continue;
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e))
                continue;
            if (p_ecs.GetComponent<NameTag>(e).GetNameTag() != "painSpot")
                continue;

            auto& painTransform = p_ecs.GetComponent<Transform>(e);
            const vec2D painPos = painTransform.GetPos();
            const vec2D painScale = painTransform.GetScale();
            const float painRadius = std::max(std::abs(painScale.x), std::abs(painScale.y)) * 0.5f;

            if (distance(targetPos, painPos) <= targetRadius + painRadius)
            {
                outSpot = e;
                return true;
            }
        }

        return false;
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
