/*!------------------------------------------------------------------------------------------------
    File: EncryptionKey.cpp
    Project: NocturnalStudios

    Brief:
        Handles the hiding and revealing of encryption keyss

    Primary Author:
         Nur Syazliyarni (100%)
        - Created logic script for keys

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "Scripts/EncryptionKey.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "GUI/GUI.h"
#include <algorithm>
#include <cctype>

namespace NocturnalEngine
{
    // Cache input singleton.
    EncryptionKey::EncryptionKey()
        : mInput{ InputSystem::GetInstance().get() }
    {
    }

    // Initialize key state when scene starts.
    void EncryptionKey::Start()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        mOriginalScale = p_ecs.GetComponent<Transform>(entityID).GetScale();
        FindEnvironment();
        if (!mHasEnvironment)
            return;

        // Place key at random spot, then start hidden.
        auto& keyT = p_ecs.GetComponent<Transform>(entityID);
        mHiddenPos = RandomPointInsideEnvironment();
        keyT.SetPos(mHiddenPos.x, mHiddenPos.y);
        keyT.SetScale(0.0f, 0.0f);

        mCollected = IsCollected(entityID);
        mRevealed = false;
    }

    // Two-step interaction:
    // - click near hidden spot -> reveal
    // - click on visible key -> collect
    void EncryptionKey::Update(float dt)
    {
        UNREFERENCED_PARAMETER(dt);
        if (!mHasEnvironment || mCollected)
            return;

        if (!mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
            return;

        // First click near hidden position reveals the key.
        if (!mRevealed)
        {
            if (IsMouseNearHiddenSpot())
            {
                auto& keyT = p_ecs.GetComponent<Transform>(entityID);
                keyT.SetPos(mHiddenPos.x, mHiddenPos.y);
                keyT.SetScale(mOriginalScale.x, mOriginalScale.y);
                mRevealed = true;
            }
            return;
        }

        // Second click on revealed key collects it.
        if (IsMouseOverKey())
        {
            CollectKey();
        }
    }

    // Find first environment object with Transform + Collider bounds.
    void EncryptionKey::FindEnvironment()
    {
        mEnvironment = 0;
        mHasEnvironment = false;

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e) || !p_ecs.HaveComponent<Collider>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();
            for (char& ch : tag)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (tag.find("environment") != std::string::npos)
            {
                mEnvironment = e;
                mHasEnvironment = true;
                return;
            }
        }
    }

    // Compute key half-size used to keep full key inside environment bounds.
    vec2D EncryptionKey::GetKeyHalfExtents()
    {
        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& c = p_ecs.GetComponent<Collider>(entityID)[0];
            if (c.mCol == Collider::ColliderType::circle ||
                c.mCol == Collider::ColliderType::bubble ||
                c.mCol == Collider::ColliderType::bossball)
            {
                const float r = std::max(c.mRadius, 0.001f);
                return { r, r };
            }
            if (c.mCol == Collider::ColliderType::rect || c.mCol == Collider::ColliderType::box)
            {
                const float hx = std::max(std::max(std::abs(c.mMin.x), std::abs(c.mMax.x)), 0.001f);
                const float hy = std::max(std::max(std::abs(c.mMin.y), std::abs(c.mMax.y)), 0.001f);
                return { hx, hy };
            }
        }

        const float hx = std::max(std::abs(mOriginalScale.x) * 0.5f, 0.001f);
        const float hy = std::max(std::abs(mOriginalScale.y) * 0.5f, 0.001f);
        return { hx, hy };
    }

    // Randomly pick one point inside environment collider while respecting key size.
    vec2D EncryptionKey::RandomPointInsideEnvironment()
    {
        auto& envT = p_ecs.GetComponent<Transform>(mEnvironment);
        auto& envCol = p_ecs.GetComponent<Collider>(mEnvironment)[0];
        const vec2D keyHalf = GetKeyHalfExtents();

        vec2D minP = envT.GetMinPos();
        vec2D maxP = envT.GetMaxPos();

        if (envCol.mCol == Collider::ColliderType::rect || envCol.mCol == Collider::ColliderType::box)
        {
            const vec2D center{ envT.GetPos().x + envCol.mOffset.x, envT.GetPos().y + envCol.mOffset.y };
            minP = { center.x - envCol.mMin.x, center.y - envCol.mMin.y };
            maxP = { center.x + envCol.mMax.x, center.y + envCol.mMax.y };
        }
        else if (envCol.mCol == Collider::ColliderType::circle ||
                 envCol.mCol == Collider::ColliderType::bubble ||
                 envCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ envT.GetPos().x + envCol.mOffset.x, envT.GetPos().y + envCol.mOffset.y };
            minP = { center.x - envCol.mRadius, center.y - envCol.mRadius };
            maxP = { center.x + envCol.mRadius, center.y + envCol.mRadius };
        }

        minP.x += keyHalf.x;
        minP.y += keyHalf.y;
        maxP.x -= keyHalf.x;
        maxP.y -= keyHalf.y;

        if (maxP.x <= minP.x) { const float mid = (minP.x + maxP.x) * 0.5f; minP.x = maxP.x = mid; }
        if (maxP.y <= minP.y) { const float mid = (minP.y + maxP.y) * 0.5f; minP.y = maxP.y = mid; }

        std::uniform_real_distribution<float> distX(minP.x, maxP.x);
        std::uniform_real_distribution<float> distY(minP.y, maxP.y);
        vec2D candidate{ distX(mRng), distY(mRng) };

        if (envCol.mCol == Collider::ColliderType::circle ||
            envCol.mCol == Collider::ColliderType::bubble ||
            envCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ envT.GetPos().x + envCol.mOffset.x, envT.GetPos().y + envCol.mOffset.y };
            const float keyRadius = std::max(keyHalf.x, keyHalf.y);
            const float allowedRadius = std::max(0.0f, envCol.mRadius - keyRadius);

            for (int i = 0; i < 12; ++i)
            {
                if (distance(candidate, center) <= allowedRadius)
                    return candidate;
                candidate = { distX(mRng), distY(mRng) };
            }
            return center;
        }

        return candidate;
    }

    // Hit test used only after key is revealed.
    bool EncryptionKey::IsMouseOverKey()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return false;

        vec2D mousePos{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        auto& transform = p_ecs.GetComponent<Transform>(entityID);

        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& collider = p_ecs.GetComponent<Collider>(entityID);
            for (int i = 0; i < 2; ++i)
            {
                vec2D offsetPos{ transform.GetPos().x + collider[i].mOffset.x,
                                 transform.GetPos().y + collider[i].mOffset.y };

                switch (collider[i].mCol)
                {
                case Collider::ColliderType::circle:
                case Collider::ColliderType::bubble:
                case Collider::ColliderType::bossball:
                    if (distance(mousePos, offsetPos) < collider[i].mRadius) return true;
                    break;
                case Collider::ColliderType::rect:
                case Collider::ColliderType::box:
                {
                    vec2D minPt{ offsetPos.x - collider[i].mMin.x, offsetPos.y - collider[i].mMin.y };
                    vec2D maxPt{ offsetPos.x + collider[i].mMax.x, offsetPos.y + collider[i].mMax.y };
                    if (mousePos.x > minPt.x && mousePos.x < maxPt.x &&
                        mousePos.y > minPt.y && mousePos.y < maxPt.y)
                        return true;
                    break;
                }
                default:
                    break;
                }
            }
            return false;
        }

        // Fallback: use transform AABB if key has no collider.
        vec2D minPt = transform.GetMinPos();
        vec2D maxPt = transform.GetMaxPos();
        return mousePos.x > minPt.x && mousePos.x < maxPt.x &&
               mousePos.y > minPt.y && mousePos.y < maxPt.y;
    }

    // Hidden key reveal check.
    bool EncryptionKey::IsMouseNearHiddenSpot() const
    {
        vec2D mouse{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        return distance(mouse, mHiddenPos) <= mRevealRadius;
    }

    // Shared collected-list lookup.
    bool EncryptionKey::IsCollected(Entity key) const
    {
        return std::find(sCollectedKeys.begin(), sCollectedKeys.end(), key) != sCollectedKeys.end();
    }

    // Add key to shared inventory if capacity allows, then hide it.
    void EncryptionKey::CollectKey()
    {
        if (mCollected)
            return;

        if (!IsCollected(entityID) && sCollectedKeys.size() < kMaxKeys)
        {
            sCollectedKeys.push_back(entityID);
        }

        if (p_ecs.HaveComponent<Transform>(entityID))
            p_ecs.GetComponent<Transform>(entityID).SetScale(0.0f, 0.0f);

        mCollected = true;
        mRevealed = false;
    }
}
