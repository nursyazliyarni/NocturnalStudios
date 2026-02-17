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

    // - Start: clear runtime tracked pain spots and offsets.
    void PainSpotSpawner::Start()
    {
        mSpots.clear();
        mOffsets.clear();
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

        Transform t;
        t.SetPos(owlTransform.GetPos().x + offset.x, owlTransform.GetPos().y + offset.y);
        t.SetScale(0.05f, 0.05f);
        t.SetRot(0.0f);
        p_ecs.AddComponent<Transform>(spot, t);

        Sprite s;
        s.SetTexture("UI/painSpot");
        s.SetLayerOrder(0);
        p_ecs.AddComponent<Sprite>(spot, s);

        NameTag tag;
        tag.SetNameTag("painSpot");
        p_ecs.AddComponent<NameTag>(spot, tag);

        mSpots.push_back(spot);
        mOffsets.push_back(offset);
    }

    // - Pick a random spot offset inside collider bounds when available, otherwise inside transform scale extents.
    vec2D PainSpotSpawner::RandomOffset(const vec2D& owlScale)
    {
        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& collider = p_ecs.GetComponent<Collider>(entityID);
            auto& col = collider[0];

            if (col.mCol == Collider::ColliderType::circle ||
                col.mCol == Collider::ColliderType::bubble ||
                col.mCol == Collider::ColliderType::bossball)
            {
                std::uniform_real_distribution<float> dist(-col.mRadius, col.mRadius);
                return vec2D(dist(mRng) + col.mOffset.x, dist(mRng) + col.mOffset.y);
            }

            if (col.mCol == Collider::ColliderType::rect || col.mCol == Collider::ColliderType::box)
            {
                std::uniform_real_distribution<float> distX(-col.mMin.x, col.mMax.x);
                std::uniform_real_distribution<float> distY(-col.mMin.y, col.mMax.y);
                return vec2D(distX(mRng) + col.mOffset.x, distY(mRng) + col.mOffset.y);
            }
        }

        std::uniform_real_distribution<float> distX(-0.5f * owlScale.x, 0.5f * owlScale.x);
        std::uniform_real_distribution<float> distY(-0.5f * owlScale.y, 0.5f * owlScale.y);
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
