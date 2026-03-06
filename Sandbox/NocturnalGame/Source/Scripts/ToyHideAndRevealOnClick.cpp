#include "pch.h"
#include "Scripts/ToyHideAndRevealOnClick.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Tag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "GUI/GUI.h"
#include "Math/Vmath.h"
#include <algorithm>
#include <cctype>

namespace NocturnalEngine
{
    ToyHideAndRevealOnClick::ToyHideAndRevealOnClick()
        : mInput{ InputSystem::GetInstance().get() }
    {
    }

    void ToyHideAndRevealOnClick::Start()
    {
        // This script is attached to the Toy entity.
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        auto& toyT = p_ecs.GetComponent<Transform>(entityID);
        mOriginalScale = toyT.GetScale();

        // Cache original collider so we can disable while hidden and restore on reveal.
        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            mHadCollider = true;
            mOriginalCollider = p_ecs.GetComponent<Collider>(entityID);
        }

        FindBG();
        if (!mHasBG)
            return;

        // Pick one random hidden position bounded by Environment collider.
        mHiddenPos = RandomPointInsideBG();

        // Hide toy and move it to its hidden position.
        toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
        toyT.SetScale(0.0f, 0.0f);

        // Disable collider so hidden toy cannot be interacted with.
        if (mHadCollider)
        {
            auto& c = p_ecs.GetComponent<Collider>(entityID);
            for (int i = 0; i < 2; ++i)
            {
                c[i].mRadius = 0.0f;
                c[i].mMin = { 0.0f, 0.0f };
                c[i].mMax = { 0.0f, 0.0f };
            }
        }

        mHidden = true;
    }

    void ToyHideAndRevealOnClick::Update(float dt)
    {
        UNREFERENCED_PARAMETER(dt);
        if (!mHasBG || !mHidden)
            return;

        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && IsMouseNearHiddenSpot())
        {
            // Reveal toy at the hidden position.
            auto& toyT = p_ecs.GetComponent<Transform>(entityID);
            toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
            toyT.SetScale(mOriginalScale.x, mOriginalScale.y);

            if (mHadCollider)
                p_ecs.GetComponent<Collider>(entityID) = mOriginalCollider;

            mHidden = false;
        }
    }

    void ToyHideAndRevealOnClick::End()
    {
        if (p_ecs.HaveComponent<Transform>(entityID))
        {
            auto& toyT = p_ecs.GetComponent<Transform>(entityID);
            toyT.SetScale(mOriginalScale.x, mOriginalScale.y);
        }
        if (mHadCollider && p_ecs.HaveComponent<Collider>(entityID))
            p_ecs.GetComponent<Collider>(entityID) = mOriginalCollider;
    }

    void ToyHideAndRevealOnClick::FindBG()
    {
        mBG = 0;
        mHasBG = false;

        // Use explicit Environment object bounds (requires Transform + Collider).
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e) || !p_ecs.HaveComponent<Collider>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();
            for (char& ch : tag)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (tag.find("environment") != std::string::npos)
            {
                mBG = e;
                mHasBG = true;
                return;
            }
        }
    }

    vec2D ToyHideAndRevealOnClick::RandomPointInsideBG()
    {
        auto& bgT = p_ecs.GetComponent<Transform>(mBG);
        auto& bgCol = p_ecs.GetComponent<Collider>(mBG)[0];
        const vec2D toyHalf = GetToyHalfExtents();

        // Start with transform bounds as fallback.
        vec2D minP = bgT.GetMinPos();
        vec2D maxP = bgT.GetMaxPos();

        // Prefer collider bounds for environment area.
        if (bgCol.mCol == Collider::ColliderType::rect || bgCol.mCol == Collider::ColliderType::box)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            minP = { center.x - bgCol.mMin.x, center.y - bgCol.mMin.y };
            maxP = { center.x + bgCol.mMax.x, center.y + bgCol.mMax.y };
        }
        else if (bgCol.mCol == Collider::ColliderType::circle ||
                 bgCol.mCol == Collider::ColliderType::bubble ||
                 bgCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            minP = { center.x - bgCol.mRadius, center.y - bgCol.mRadius };
            maxP = { center.x + bgCol.mRadius, center.y + bgCol.mRadius };
        }

        // Shrink by toy half-size so full toy stays inside bounds.
        minP.x += toyHalf.x;
        minP.y += toyHalf.y;
        maxP.x -= toyHalf.x;
        maxP.y -= toyHalf.y;

        if (maxP.x <= minP.x)
        {
            const float mid = (minP.x + maxP.x) * 0.5f;
            minP.x = maxP.x = mid;
        }
        if (maxP.y <= minP.y)
        {
            const float mid = (minP.y + maxP.y) * 0.5f;
            minP.y = maxP.y = mid;
        }

        std::uniform_real_distribution<float> distX(minP.x, maxP.x);
        std::uniform_real_distribution<float> distY(minP.y, maxP.y);
        vec2D candidate{ distX(mRng), distY(mRng) };

        // For circular environment collider, reject candidates outside allowed circle.
        if (bgCol.mCol == Collider::ColliderType::circle ||
            bgCol.mCol == Collider::ColliderType::bubble ||
            bgCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            const float toyRadius = std::max(toyHalf.x, toyHalf.y);
            const float allowedRadius = std::max(0.0f, bgCol.mRadius - toyRadius);

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

    vec2D ToyHideAndRevealOnClick::GetToyHalfExtents()
    {
        // Prefer collider dimensions when available.
        if (mHadCollider)
        {
            const auto& c = mOriginalCollider[0];

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

        // Fallback to transform scale.
        const float hx = std::max(std::abs(mOriginalScale.x) * 0.5f, 0.001f);
        const float hy = std::max(std::abs(mOriginalScale.y) * 0.5f, 0.001f);
        return { hx, hy };
    }

    bool ToyHideAndRevealOnClick::IsMouseNearHiddenSpot() const
    {
        vec2D mouse{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        return distance(mouse, mHiddenPos) <= mRevealRadius;
    }
}
