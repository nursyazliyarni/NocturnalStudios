/*!------------------------------------------------------------------------------------------------
    File: PainSpotSpawner.cpp
    Project: NocturnalStudio

    Brief:
        Spawns painSpot entities as the owl's health decreases.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created the PainSpotSpawner script and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/PainSpotSpawner.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Transform.h"

namespace NocturnalEngine
{
    // - Constructor: set health thresholds and initialize RNG.
    PainSpotSpawner::PainSpotSpawner()
        : mThresholds{ 0.75, 0.50, 0.25, 0.10 }
        , mRng{ std::random_device{}() }
    {
    }

    // - Start: clear runtime tracked spots and hide editor-authored pain spot placeholders at play start.
    void PainSpotSpawner::Start()
    {
        mSpots.clear();
        mOffsets.clear();
        mHasSpotTemplate = false;
        mTemplateHasCollider = false;

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e))
                continue;
            if (p_ecs.GetComponent<NameTag>(e).GetNameTag() != "painSpot")
                continue;

            if (!mHasSpotTemplate && p_ecs.HaveComponent<Sprite>(e))
            {
                mSpotTemplateTransform = p_ecs.GetComponent<Transform>(e);
                mSpotTemplateSprite = p_ecs.GetComponent<Sprite>(e);
                mHasSpotTemplate = true;

                if (p_ecs.HaveComponent<Collider>(e))
                {
                    mSpotTemplateCollider = p_ecs.GetComponent<Collider>(e);
                    mTemplateHasCollider = true;
                }
            }

            auto& t = p_ecs.GetComponent<Transform>(e);
            t.SetScale(0.0f, 0.0f);
        }
    }

    // - Update: remove invalid tracked spots, spawn new spots at health thresholds, and keep spots following the animal.
    void PainSpotSpawner::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (!p_ecs.HaveComponent<AnimalAttributes>(entityID) || !p_ecs.HaveComponent<Transform>(entityID))
            return;

        CleanupDestroyedSpots();

        auto& attrs = p_ecs.GetComponent<AnimalAttributes>(entityID);
        const double maxHealth = attrs.maxHealth > 0.0 ? attrs.maxHealth : 1.0;
        const double healthRatio = attrs.current_health / maxHealth;

        while (mSpots.size() < mThresholds.size() && healthRatio <= mThresholds[mSpots.size()])
        {
            SpawnSpot();
        }

        UpdateSpotPositions();
    }

    // - Spawn one pain spot entity and remember its local offset from animal.
    void PainSpotSpawner::SpawnSpot()
    {
        auto& owlTransform = p_ecs.GetComponent<Transform>(entityID);
        const vec2D offset = RandomOffset(owlTransform.GetScale());

        Entity spot = p_ecs.CreateEntity();

        Transform t = mHasSpotTemplate ? mSpotTemplateTransform : Transform{};
        t.SetPos(owlTransform.GetPos().x + offset.x, owlTransform.GetPos().y + offset.y);
        if (!mHasSpotTemplate)
        {
            t.SetScale(0.05f, 0.05f);
            t.SetRot(0.0f);
        }
        p_ecs.AddComponent<Transform>(spot, t);

        Sprite s = mHasSpotTemplate ? mSpotTemplateSprite : Sprite{};
        if (!mHasSpotTemplate)
        {
            s.SetTexture("UI/painSpot");

        }
        s.SetLayerOrder(6);
        p_ecs.AddComponent<Sprite>(spot, s);

        if (mTemplateHasCollider)
        {
            p_ecs.AddComponent<Collider>(spot, mSpotTemplateCollider);
        }

        NameTag tag;
        tag.SetNameTag("painSpot");
        p_ecs.AddComponent<NameTag>(spot, tag);

        mSpots.push_back(spot);
        mOffsets.push_back(offset);
    }

    // - Pick a random spot offset inside collider bounds when available, otherwise inside transform scale extents.
    vec2D PainSpotSpawner::RandomOffset(const vec2D& owlScale)
    {
        vec2D spotOffset(0.0f, 0.0f);
        float spotHalfX = std::abs(owlScale.x) * 0.025f;
        float spotHalfY = std::abs(owlScale.y) * 0.025f;
        float spotRadius = std::max(spotHalfX, spotHalfY);

        if (mHasSpotTemplate)
        {
            const vec2D templateScale = mSpotTemplateTransform.GetScale();
            spotHalfX = std::max(std::abs(templateScale.x) * 0.5f, 0.001f);
            spotHalfY = std::max(std::abs(templateScale.y) * 0.5f, 0.001f);
            spotRadius = std::max(spotHalfX, spotHalfY);
        }

        if (mTemplateHasCollider)
        {
            const auto& spotCol = mSpotTemplateCollider[0];
            spotOffset = spotCol.mOffset;

            if (spotCol.mCol == Collider::ColliderType::circle ||
                spotCol.mCol == Collider::ColliderType::bubble ||
                spotCol.mCol == Collider::ColliderType::bossball)
            {
                spotRadius = std::max(spotCol.mRadius, 0.001f);
                spotHalfX = spotRadius;
                spotHalfY = spotRadius;
            }
            else if (spotCol.mCol == Collider::ColliderType::rect || spotCol.mCol == Collider::ColliderType::box)
            {
                spotHalfX = std::max(std::max(std::abs(spotCol.mMin.x), std::abs(spotCol.mMax.x)), 0.001f);
                spotHalfY = std::max(std::max(std::abs(spotCol.mMin.y), std::abs(spotCol.mMax.y)), 0.001f);
                spotRadius = std::max(spotHalfX, spotHalfY);
            }
        }

        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& collider = p_ecs.GetComponent<Collider>(entityID);
            auto& col = collider[0];

            if (col.mCol == Collider::ColliderType::circle ||
                col.mCol == Collider::ColliderType::bubble ||
                col.mCol == Collider::ColliderType::bossball)
            {
                float available = col.mRadius - spotRadius;
                if (available < 0.0f)
                    available = 0.0f;

                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
                std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);
                const float angle = angleDist(mRng);
                const float radius = std::sqrt(radiusDist(mRng)) * available;
                const vec2D centerLocal(std::cos(angle) * radius, std::sin(angle) * radius);

                return vec2D(col.mOffset.x + centerLocal.x - spotOffset.x,
                             col.mOffset.y + centerLocal.y - spotOffset.y);
            }

            if (col.mCol == Collider::ColliderType::rect || col.mCol == Collider::ColliderType::box)
            {
                float minX = -col.mMin.x + spotHalfX;
                float maxX = col.mMax.x - spotHalfX;
                float minY = -col.mMin.y + spotHalfY;
                float maxY = col.mMax.y - spotHalfY;

                if (minX > maxX) { minX = maxX = (minX + maxX) * 0.5f; }
                if (minY > maxY) { minY = maxY = (minY + maxY) * 0.5f; }

                std::uniform_real_distribution<float> distX(minX, maxX);
                std::uniform_real_distribution<float> distY(minY, maxY);
                const vec2D centerLocal(distX(mRng), distY(mRng));

                return vec2D(col.mOffset.x + centerLocal.x - spotOffset.x,
                             col.mOffset.y + centerLocal.y - spotOffset.y);
            }
        }

        const float halfOwlX = std::abs(owlScale.x) * 0.5f;
        const float halfOwlY = std::abs(owlScale.y) * 0.5f;
        float minX = -halfOwlX + spotHalfX;
        float maxX = halfOwlX - spotHalfX;
        float minY = -halfOwlY + spotHalfY;
        float maxY = halfOwlY - spotHalfY;

        if (minX > maxX) { minX = maxX = 0.0f; }
        if (minY > maxY) { minY = maxY = 0.0f; }

        std::uniform_real_distribution<float> distX(minX, maxX);
        std::uniform_real_distribution<float> distY(minY, maxY);
        return vec2D(distX(mRng), distY(mRng));
    }

    // - Remove tracked entries if the underlying entity was destroyed or is no longer a pain spot.
    void PainSpotSpawner::CleanupDestroyedSpots()
    {
        size_t i = 0;
        while (i < mSpots.size())
        {
            if (!p_ecs.HaveComponent<Transform>(mSpots[i]) || !p_ecs.HaveComponent<NameTag>(mSpots[i]) ||
                p_ecs.GetComponent<NameTag>(mSpots[i]).GetNameTag() != "painSpot")
            {
                mSpots.erase(mSpots.begin() + i);
                mOffsets.erase(mOffsets.begin() + i);
                continue;
            }
            ++i;
        }
    }

    // - Keep spawned pain spots attached to the animal as it moves.
    void PainSpotSpawner::UpdateSpotPositions()
    {
        auto& owlTransform = p_ecs.GetComponent<Transform>(entityID);
        for (size_t i = 0; i < mSpots.size(); ++i)
        {
            if (!p_ecs.HaveComponent<Transform>(mSpots[i]))
                continue;

            auto& spotTransform = p_ecs.GetComponent<Transform>(mSpots[i]);
            spotTransform.SetPos(owlTransform.GetPos().x + mOffsets[i].x,
                                 owlTransform.GetPos().y + mOffsets[i].y);
        }
    }
}
