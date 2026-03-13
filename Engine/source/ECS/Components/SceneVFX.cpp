/*!------------------------------------------------------------------------------------------------
    File: SceneVFX.cpp
    Project: NocturnalStudio

    Brief:
        This file contains the implementation of the SceneVFX component.

    Primary Author:
         Rishika Bimal Attodi
        - Added scene-level VFX component for JSON-driven scene effects.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/SceneVFX.h"

namespace NocturnalEngine
{
    SceneVFX::SceneVFX()
        : mParticlesEnabled(false), mParticleType(ParticleType::None), mRenderAfterLayer(0)
    {
    }

    bool SceneVFX::ToEditor()
    {
        return false;
    }

    bool SceneVFX::Deserialize(const rapidjson::Value& obj)
    {
        if (obj.HasMember("ParticlesEnabled") && obj["ParticlesEnabled"].IsBool())
            mParticlesEnabled = obj["ParticlesEnabled"].GetBool();

        if (obj.HasMember("ParticleType") && obj["ParticleType"].IsInt())
            mParticleType = static_cast<ParticleType>(obj["ParticleType"].GetInt());

        if (obj.HasMember("RenderAfterLayer") && obj["RenderAfterLayer"].IsInt())
            mRenderAfterLayer = obj["RenderAfterLayer"].GetInt();

        return true;
    }

    bool SceneVFX::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
        writer->Key("ParticlesEnabled");
        writer->Bool(mParticlesEnabled);

        writer->Key("ParticleType");
        writer->Int(static_cast<int>(mParticleType));

        writer->Key("RenderAfterLayer");
        writer->Int(mRenderAfterLayer);

        return true;
    }
}