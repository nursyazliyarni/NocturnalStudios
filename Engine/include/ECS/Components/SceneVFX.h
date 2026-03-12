/*!------------------------------------------------------------------------------------------------
    File: SceneVFX.h
    Project: NocturnalStudio

    Brief:
        This file contains the component declaration for scene-level visual effects.
        It is used to control effects such as floating particle overlays through JSON.

    Primary Author:
         Rishika Bimal Attodi
        - Added scene-level VFX component for JSON-driven scene effects.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"

namespace NocturnalEngine
{
    class SceneVFX : public IComponent
    {
    public:
        enum class ParticleType
        {
            None = 0,
            FloatingDots = 1
        };

        SceneVFX();
        ~SceneVFX() = default;

        bool ToEditor() override;
        virtual bool Deserialize(const rapidjson::Value& obj) override;
        virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override;

        bool& GetParticlesEnabled() { return mParticlesEnabled; }
        void SetParticlesEnabled(bool enabled) { mParticlesEnabled = enabled; }

        ParticleType GetParticleType() const { return mParticleType; }
        void SetParticleType(ParticleType type) { mParticleType = type; }

        int& GetRenderAfterLayer() { return mRenderAfterLayer; }
        void SetRenderAfterLayer(int layer) { mRenderAfterLayer = layer; }

        Entity& GetComponentEntityID() override { return entityID; }
        void SetComponentEntityID(Entity& entity) override { entityID = entity; }

    private:
        bool mParticlesEnabled{};
        ParticleType mParticleType{ ParticleType::None };

        // Draw particle pass after this layer is rendered.
        // Example:
        // 0 = draw above background layer 0, but below layer 1 and above.
        int mRenderAfterLayer{ 0 };
    };
}