/*!------------------------------------------------------------------------------------------------
    File: Text.cpp
    Project: NocturnalStudio

    Brief:
        This file is used to serialize and deserialize text components for ECS.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Text.h"

namespace NocturnalEngine
{
    Text::Text()
        : mContent{ "New Text" }
        , mSize{ 24.0f }
        , mWorldSpace{ true }
        , mCenter{ false }
    {
        mColor[0] = 1.0f;
        mColor[1] = 1.0f;
        mColor[2] = 1.0f;
        mColor[3] = 1.0f;
    }

    bool Text::Deserialize(const rapidjson::Value& obj)
    {
        mContent = obj["Content"].GetString();
        mSize = obj["Size"].GetFloat();

        const rapidjson::Value& col = obj["Color"];
        mColor[0] = col[0].GetFloat();
        mColor[1] = col[1].GetFloat();
        mColor[2] = col[2].GetFloat();
        mColor[3] = col[3].GetFloat();

        mWorldSpace = obj["WorldSpace"].GetBool();
        mCenter = obj["Center"].GetBool();

        return true;
    }

    bool Text::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
        writer->Key("Content");
        writer->String(mContent.c_str());

        writer->Key("Size");
        writer->Double(mSize);

        writer->Key("Color");
        writer->StartArray();
        writer->Double(mColor[0]);
        writer->Double(mColor[1]);
        writer->Double(mColor[2]);
        writer->Double(mColor[3]);
        writer->EndArray();

        writer->Key("WorldSpace");
        writer->Bool(mWorldSpace);

        writer->Key("Center");
        writer->Bool(mCenter);

        return true;
    }
}
