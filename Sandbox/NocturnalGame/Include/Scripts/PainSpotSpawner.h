/*!------------------------------------------------------------------------------------------------
    File: PainSpotSpawner.h
    Project: NocturnalStudio

    Brief:
        Spawns painSpot entities as the owl's health decreases.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created the PainSpotSpawner script and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Scripts/IScript.h"
#include "ECS/Components/AnimalAttributes.h"

namespace NocturnalEngine
{
    class PainSpotSpawner : public IScript
    {
    public:
        PainSpotSpawner();
        ~PainSpotSpawner() = default;

        virtual PainSpotSpawner* Clone() const override { return new PainSpotSpawner(*this); }
        // Clears runtime pain spot tracking when scene starts.
        virtual void Start() override;
        // Spawns and updates pain spots based on current health percentage.
        virtual void Update(float Frametime) override;
        virtual void End() override {}

        bool Requirements(Entity e) override {
            if (p_ecs.HaveComponent<AnimalAttributes>(e))return true;
            return false;
        }

        virtual std::string GetScriptName() override { return "PainSpotSpawner"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        std::vector<Entity> mSpots;
        std::vector<vec2D> mOffsets;
        std::array<double, 4> mThresholds;

        std::mt19937 mRng;

        // Creates one pain spot entity and stores its offset from the animal.
        void SpawnSpot();
        // Picks a random local offset inside collider bounds (or scale fallback).
        vec2D RandomOffset(const vec2D& owlScale);
        // Removes destroyed pain spots from local tracking arrays.
        void CleanupDestroyedSpots();
        // Repositions all tracked pain spots to follow the animal transform.
        void UpdateSpotPositions();
    };
}
