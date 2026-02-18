/*!------------------------------------------------------------------------------------------------
    File: Text.cpp
    Project: NocturnalStudio

    Brief:
        This file is used to serialize and deserialize text components for ECS.

    Primary Author:
         Nur Syazliyarni (100%)
        - Created the tag class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Text.h"
#include "ResourceManager/ResourceManager.h"
#include <imgui.h>

namespace NocturnalEngine
{
    /*!*************************************************************************
    Ctor for Text Component
    ****************************************************************************/
    Text::Text()
        : mContent{ "New Text" }
        , mFontName{ "KernlGrotesk" }
        , mSize{ 24.0f }
        , mColor{ 1.0f, 1.0f, 1.0f, 1.0f }
        , mWorldSpace{ false }
        , mCenter{ false }
        , mPosition{ 0.0f, 0.0f }
        , line_height{0}
    {
    }

    void Text::ToEditor() {
        if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_None))
        {

            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), GetContent().c_str());
            if (ImGui::InputTextMultiline("Content", buffer, sizeof(buffer), ImVec2(0, 80)))
            {
                SetContent(std::string(buffer));
            }

            auto& fontName = GetFont();
            ImGui::Text("Font"); ImGui::SameLine();
            ImGui::SetNextItemWidth(180.0f);
            if (ImGui::BeginCombo("##textfont", fontName.c_str()))
            {
                for (auto& [name, font] : ResourceManager::fonts)
                {
                    bool selected = (name == fontName);
                    if (ImGui::Selectable(name.c_str(), selected))
                    {
                        fontName = name;
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::DragFloat("Size", &GetSize(), 0.1f, 1.0f, 200.0f);
            ImGui::ColorEdit4("Color", GetColor());
            ImGui::Checkbox("World Space", &GetWorldSpace());
            ImGui::Checkbox("Center", &GetCenter());

            ImGui::Text("Position");
            ImGui::Text("X"); ImGui::SameLine();
            ImGui::DragFloat("##TextPosX", &GetPosition()[0], 0.01f);
            ImGui::Text("Y"); ImGui::SameLine();
            ImGui::DragFloat("##TextPosY", &GetPosition()[1], 0.01f);
            ImGui::Text("Rotation"); ImGui::SameLine();
            ImGui::DragFloat("##TextRot", &Rot(), 5.f, 0.f, 360.f);
            ImGui::Text("Line Height"); ImGui::SameLine();
            ImGui::DragInt("##Line Height", &Get_line_height(), 1);
        }
    }

    /*!*************************************************************************
    Deserialize for Text Component
    ****************************************************************************/
    bool Text::Deserialize(const rapidjson::Value& obj)
    {
        if (obj.HasMember("Content") && obj["Content"].IsString())
            mContent = obj["Content"].GetString();

        if (obj.HasMember("Font") && obj["Font"].IsString())
            mFontName = obj["Font"].GetString();

        if (obj.HasMember("Size") && obj["Size"].IsNumber())
            mSize = obj["Size"].GetFloat();

        if (obj.HasMember("ColorR") && obj["ColorR"].IsNumber()) mColor[0] = obj["ColorR"].GetFloat();
        if (obj.HasMember("ColorG") && obj["ColorG"].IsNumber()) mColor[1] = obj["ColorG"].GetFloat();
        if (obj.HasMember("ColorB") && obj["ColorB"].IsNumber()) mColor[2] = obj["ColorB"].GetFloat();
        if (obj.HasMember("ColorA") && obj["ColorA"].IsNumber()) mColor[3] = obj["ColorA"].GetFloat();

        if (obj.HasMember("PosX") && obj["PosX"].IsNumber()) mPosition[0] = obj["PosX"].GetFloat();
        if (obj.HasMember("PosY") && obj["PosY"].IsNumber()) mPosition[1] = obj["PosY"].GetFloat();
        if (obj.HasMember("Rotation") && obj["Rotation"].IsNumber()) rotation = obj["Rotation"].GetFloat();
        if (obj.HasMember("Line Height") && obj["Line Height"].IsInt()) line_height = obj["Line Height"].GetInt();

        if (obj.HasMember("WorldSpace") && obj["WorldSpace"].IsBool())
            mWorldSpace = obj["WorldSpace"].GetBool();

        if (obj.HasMember("Center") && obj["Center"].IsBool())
            mCenter = obj["Center"].GetBool();

        return true;
    }

    /*!*************************************************************************
    Serialize for Text Component
    ****************************************************************************/
    bool Text::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
        writer->Key("Content");
        writer->String(mContent.c_str());

        writer->Key("Font");
        writer->String(mFontName.c_str());

        writer->Key("Size");
        writer->Double(mSize);

        writer->Key("ColorR");
        writer->Double(mColor[0]);
        writer->Key("ColorG");
        writer->Double(mColor[1]);
        writer->Key("ColorB");
        writer->Double(mColor[2]);
        writer->Key("ColorA");
        writer->Double(mColor[3]);

        writer->Key("PosX");
        writer->Double(mPosition[0]);
        writer->Key("PosY");
        writer->Double(mPosition[1]);
        writer->Key("Rotation");
        writer->Double(rotation);
        writer->Key("Line Height");
        writer->Int(line_height);

        writer->Key("WorldSpace");
        writer->Bool(mWorldSpace);

        writer->Key("Center");
        writer->Bool(mCenter);

        return true;
    }
}
