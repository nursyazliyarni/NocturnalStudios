/*!------------------------------------------------------------------------------------------------
    File: Text.h
    Project: NocturnalStudio

    Brief:
        A Component used for rendering text (world-space or screen-space).

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include <string>

namespace NocturnalEngine
{
    class Text : public IComponent
    {
    public:
        Text();
        ~Text() = default;

        void SetContent(const std::string& text) { mContent = text; }
        std::string& GetContent() { return mContent; }

        void SetSize(float size) { mSize = size; }
        float& GetSize() { return mSize; }

        // RGBA
        float* GetColor() { return mColor; }
        void SetColor(float r, float g, float b, float a)
        {
            mColor[0] = r; mColor[1] = g; mColor[2] = b; mColor[3] = a;
        }

        void SetWorldSpace(bool ws) { mWorldSpace = ws; }
        bool& GetWorldSpace() { return mWorldSpace; }

        void SetCenter(bool c) { mCenter = c; }
        bool& GetCenter() { return mCenter; }

        virtual bool Deserialize(const rapidjson::Value& obj);
        virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

        Entity& GetComponentEntityID() { return entityID; }
        void SetComponentEntityID(Entity& entity) { entityID = entity; }

    private:
        std::string mContent;
        float mSize;

        float mColor[4];   // r,g,b,a
        bool mWorldSpace;
        bool mCenter;
    };
}
