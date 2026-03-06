/*!------------------------------------------------------------------------------------------------
    File: PainSpotDuplicateSpawner.h
    Project: NocturnalStudio

    Brief:
        Spawns pain spots by duplicating a scene-authored pain spot template.

    Primary Author:
         Nur Syazliyarni
        - Created duplicate-based pain spot spawner script.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Scripts/IScript.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Transform.h"

namespace NocturnalEngine
{
    // Spawns runtime pain spots by cloning one scene-authored pain spot template.
    // Attach this script to an animal entity that has AnimalAttributes.
    class PainSpotDuplicateSpawner : public IScript
    {
    public:
        PainSpotDuplicateSpawner();
        ~PainSpotDuplicateSpawner() = default;

        virtual PainSpotDuplicateSpawner* Clone() const override { return new PainSpotDuplicateSpawner(*this); }
        virtual void Start() override;
        virtual void Update(float Frametime) override;
        virtual void End() override {}

        bool Requirements(Entity e) override {
            if (p_ecs.HaveComponent<AnimalAttributes>(e)) return true;
            return false;
        }

        virtual std::string GetScriptName() override { return "PainSpotDuplicateSpawner"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        // Runtime-tracked spawned spot entities and their local offsets from the animal.
        std::vector<Entity> mSpots;
        std::vector<vec2D> mOffsets;
        // Health thresholds at which each additional pain spot appears.
        std::array<double, 4> mThresholds;
        std::mt19937 mRng;

        // Cached template data copied from the scene's "PainSpot"/"painSpot" entity.
        bool mHasSpotTemplate{ false };
        bool mTemplateHasCollider{ false };
        Transform mSpotTemplateTransform{};
        Sprite mSpotTemplateSprite{};
        Collider mSpotTemplateCollider{};

        // Finds and caches the first valid pain spot template in the current scene.
        void CachePainSpotTemplate();
        // Accept both naming styles used across scenes.
        bool IsPainSpotName(const std::string& name) const;
        // Creates one new pain spot entity from cached template data.
        void SpawnSpot();
        // Picks a safe local offset so the spawned spot stays inside the animal collider.
        vec2D RandomOffset(const vec2D& animalScale);
        // Removes tracking entries for destroyed/non-painSpot entities.
        void CleanupDestroyedSpots();
        // Keeps all spawned spots attached to the animal as it moves.
        void UpdateSpotPositions();
    };
}
