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
#include "IScript.h"

namespace NocturnalEngine
{
    class PainSpotSpawner : public IScript
    {
    public:
        PainSpotSpawner();
        ~PainSpotSpawner() = default;

        virtual PainSpotSpawner* Clone() const override { return new PainSpotSpawner(*this); }
        virtual void Start() override;
        virtual void Update(float Frametime) override;
        virtual void End() override {}
        virtual std::string GetScriptName() override { return "PainSpotSpawner"; }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        std::vector<Entity> mSpots;
        std::vector<vec2D> mOffsets;
        std::array<double, 4> mThresholds;

        std::mt19937 mRng;

        void SpawnSpot();
        vec2D RandomOffset(const vec2D& owlScale);
        void CleanupDestroyedSpots();
        void UpdateSpotPositions();
    };
}
