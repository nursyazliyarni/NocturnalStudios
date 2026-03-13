/*!------------------------------------------------------------------------------------------------
    File: PainSpotDuplicateSpawner.cpp
    Project: NocturnalStudios

    Brief:
        Spawns pain spots by duplicating a scene-authored pain spot template.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created duplicate-based pain spot spawner script.

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/PainSpotDuplicateSpawner.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Transform.h"

namespace NocturnalEngine
{
    // Configure spawn thresholds and random generator once when script is created.
    PainSpotDuplicateSpawner::PainSpotDuplicateSpawner()
        : mThresholds{ 0.75, 0.50, 0.25, 0.10 }
        , mRng{ std::random_device{}() }
    {
    }

    // Some scenes use "PainSpot", others use "painSpot". Support both.
    bool PainSpotDuplicateSpawner::IsPainSpotName(const std::string& name) const
    {
        return name == "PainSpot" || name == "painSpot";
    }

    // Cache one scene-authored pain spot as the clone template for runtime spawning.
    void PainSpotDuplicateSpawner::CachePainSpotTemplate()
    {
        mHasSpotTemplate = false;
        mTemplateHasCollider = false;

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e) || !p_ecs.HaveComponent<Sprite>(e))
                continue;

            auto& tag = p_ecs.GetComponent<NameTag>(e);
            if (!IsPainSpotName(tag.GetNameTag()))
                continue;

            mSpotTemplateTransform = p_ecs.GetComponent<Transform>(e);
            mSpotTemplateSprite = p_ecs.GetComponent<Sprite>(e);
            mHasSpotTemplate = true;

            if (p_ecs.HaveComponent<Collider>(e))
            {
                mSpotTemplateCollider = p_ecs.GetComponent<Collider>(e);
                mTemplateHasCollider = true;
            }
            break;
        }
    }

    // Reset runtime data, cache template, and hide editor placeholders when play starts.
    void PainSpotDuplicateSpawner::Start()
    {
        mSpots.clear();
        mOffsets.clear();
        CachePainSpotTemplate();

        // Hide editor-authored pain spot placeholders at play start.
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e))
                continue;

            auto& tag = p_ecs.GetComponent<NameTag>(e);
            if (!IsPainSpotName(tag.GetNameTag()))
                continue;

            auto& t = p_ecs.GetComponent<Transform>(e);
            t.SetScale(0.0f, 0.0f);
        }
    }

    // Spawn more spots as health drops below each threshold, then keep them following animal.
    void PainSpotDuplicateSpawner::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (!p_ecs.HaveComponent<AnimalAttributes>(entityID) || !p_ecs.HaveComponent<Transform>(entityID))
            return;
        if (!mHasSpotTemplate)
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

    // Create one runtime pain spot by duplicating cached template components.
    void PainSpotDuplicateSpawner::SpawnSpot()
    {
        auto& animalTransform = p_ecs.GetComponent<Transform>(entityID);
        const vec2D offset = RandomOffset(animalTransform.GetScale());

        Entity spot = p_ecs.CreateEntity();

        Transform t = mSpotTemplateTransform;
        t.SetPos(animalTransform.GetPos().x + offset.x, animalTransform.GetPos().y + offset.y);
        p_ecs.AddComponent<Transform>(spot, t);

        Sprite s = mSpotTemplateSprite;
        s.SetLayerOrder(5);
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

    // Choose a local offset such that the spawned pain spot collider remains inside animal collider.
    vec2D PainSpotDuplicateSpawner::RandomOffset(const vec2D& animalScale)
    {
        // Default spot footprint from template transform.
        vec2D spotOffset(0.0f, 0.0f);
        float spotHalfX = std::max(std::abs(mSpotTemplateTransform.GetScale().x) * 0.5f, 0.001f);
        float spotHalfY = std::max(std::abs(mSpotTemplateTransform.GetScale().y) * 0.5f, 0.001f);
        float spotRadius = std::max(spotHalfX, spotHalfY);

        // If template has collider, derive the footprint from collider instead of sprite scale.
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

        // Prefer animal collider bounds if available.
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

            // Rectangle/box animal collider: shrink valid range by spot footprint.
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

        // Fallback when animal has no collider: use transform scale as bounds.
        const float halfAnimalX = std::abs(animalScale.x) * 0.5f;
        const float halfAnimalY = std::abs(animalScale.y) * 0.5f;
        float minX = -halfAnimalX + spotHalfX;
        float maxX = halfAnimalX - spotHalfX;
        float minY = -halfAnimalY + spotHalfY;
        float maxY = halfAnimalY - spotHalfY;

        if (minX > maxX) { minX = maxX = 0.0f; }
        if (minY > maxY) { minY = maxY = 0.0f; }

        std::uniform_real_distribution<float> distX(minX, maxX);
        std::uniform_real_distribution<float> distY(minY, maxY);
        return vec2D(distX(mRng), distY(mRng));
    }

    // Remove tracked spots that no longer exist or are no longer tagged as pain spots.
    void PainSpotDuplicateSpawner::CleanupDestroyedSpots()
    {
        size_t i = 0;
        while (i < mSpots.size())
        {
            if (!p_ecs.HaveComponent<Transform>(mSpots[i]) || !p_ecs.HaveComponent<NameTag>(mSpots[i]) ||
                !IsPainSpotName(p_ecs.GetComponent<NameTag>(mSpots[i]).GetNameTag()))
            {
                mSpots.erase(mSpots.begin() + i);
                mOffsets.erase(mOffsets.begin() + i);
                continue;
            }
            ++i;
        }
    }

    // Keep each spawned spot at its stored local offset relative to animal position.
    void PainSpotDuplicateSpawner::UpdateSpotPositions()
    {
        auto& animalTransform = p_ecs.GetComponent<Transform>(entityID);
        for (size_t i = 0; i < mSpots.size(); ++i)
        {
            if (!p_ecs.HaveComponent<Transform>(mSpots[i]))
                continue;

            auto& spotTransform = p_ecs.GetComponent<Transform>(mSpots[i]);
            spotTransform.SetPos(animalTransform.GetPos().x + mOffsets[i].x,
                                 animalTransform.GetPos().y + mOffsets[i].y);
        }
    }
}
