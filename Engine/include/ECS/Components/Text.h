/*!------------------------------------------------------------------------------------------------
    File: Text.h
    Project: NocturnalStudio

    Brief:
        A Component used for rendering text (world-space or screen-space).

    Primary Author:
         Nur Syazliyarni (100%)
        - Added Text component support.

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
        bool ToEditor()override;

        void SetContent(const std::string& text) { mContent = text; }
        std::string& GetContent() { return mContent; }

        void SetFont(const std::string& fontName) { mFontName = fontName; }
        std::string& GetFont() { return mFontName; }

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

        float* GetPosition() { return mPosition; }
        void SetPosition(float x, float y) { mPosition[0] = x; mPosition[1] = y; }

        float& Rot() { return rotation; }

        int& Get_line_height() { return line_height; }
        void Set_line_height(int Line_Height) { line_height = Line_Height; }

        int& Get_layering_order() { return LayerOrder; }
        void Set_layering_order(int layeringOrder) { LayerOrder = layeringOrder; }

        std::vector<std::string>& GET_mContent_array() { return mContent_array; }
        const std::string& GET_current_mContent(int index) const { return mContent_array[index]; }
        std::vector<std::pair<float,float>>& GET_position_vector() { return position_vector; }
        glm::vec2 GET_current_position(int index) const { return glm::vec2(position_vector[index].first,position_vector[index].second); }

        int& Get_TEXT_type() { return TEXT_type; }
        void Set_TEXT_type(int enum_text_type) { TEXT_type = enum_text_type; }

    private:
        int TEXT_type;
        std::string mContent;
        std::vector<std::string> mContent_array;
        std::string mFontName;
        float mSize;

        float mColor[4];   // r,g,b,a
        bool mWorldSpace;
        bool mCenter;

        float mPosition[2];   // x, y
        std::vector<std::pair<float,float>> position_vector; //vector for storing pair of x and y
        float rotation{};
        int line_height;
        int LayerOrder; // layering order for rendering

        enum text_type {
            single_text, array_text, text_type_count
        };
        const char* text_type_ToString(int enum_text_type);
        static int current_index;
        friend class Graphic;
    };
}
