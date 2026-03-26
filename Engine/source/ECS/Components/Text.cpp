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
        : TEXT_type{single_text}
        , mContent{ "New Text" }
        , mFontName{ "KernlGrotesk" }
        , mSize{ 24.0f }
        , mColor{ 1.0f, 1.0f, 1.0f, 1.0f }
        , mWorldSpace{ false }
        , mCenter{ false }
        , mPosition{ 0.0f, 0.0f }
        , line_height{0}
        , LayerOrder{0}
        , visibility{true}
    {
    }

    const char* Text::text_type_ToString(int enum_text_type) {
        switch (enum_text_type) {
        case text_type::single_text: return "single text";
        case text_type::array_text: return "array of text";
        default: return "Unknown Text Type";
        }
    }

    int Text::current_index{0};

    bool Text::ToEditor() {
        bool changed = false;
        if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_None))
        {
            ImGui::Text("text type");ImGui::SameLine();
            if (ImGui::BeginCombo("##text type", text_type_ToString(TEXT_type)))
            {
                for (unsigned int i{ 0 }; i < text_type::text_type_count; i++) {
                    text_type selected_text_type = static_cast<text_type>(i);
                    bool is_selected = (TEXT_type == selected_text_type);
                    if (ImGui::Selectable(text_type_ToString(selected_text_type), is_selected))
                    {
                        TEXT_type = selected_text_type;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), GetContent().c_str());
            if (TEXT_type == text_type::single_text) {
                ImGui::Text("Content"); ImGui::SameLine();
                if (ImGui::InputTextMultiline("##Content", buffer, sizeof(buffer), ImVec2(0, 80)))
                {
                    SetContent(std::string(buffer));
                }

                ImGui::Text("Position");
                ImGui::Text("X"); ImGui::SameLine();
                ImGui::DragFloat("##TextPosX", &GetPosition()[0], 0.01f);
                ImGui::Text("Y"); ImGui::SameLine();
                ImGui::DragFloat("##TextPosY", &GetPosition()[1], 0.01f);
                //changed = true;
            }

            else { //array of text
                static char new_buffer[1024] = "", edit_buffer[1024] = "";
                ImGui::Text("New Content"); static int check_index{ -1 };
                ImGui::InputTextMultiline("##New Content", new_buffer, sizeof(new_buffer), ImVec2(0, 80));
                if (position_vector.size() != mContent_array.size()) { position_vector.resize(mContent_array.size()); }

                if (ImGui::Button("Add new content")) {
                    mContent_array.push_back(std::string(new_buffer));
                    new_buffer[0] = '\0'; //clear the content after adding operation is successful
                    position_vector.push_back({0.0f,0.0f});
                }

                if (!mContent_array.empty())
                {
                    ImGui::Text("Total current number of text inside array: %d", mContent_array.size());
                    ImGui::Text("Current Content Index:"); ImGui::SameLine();
                    ImGui::DragInt("##Current Content Index", &current_index, 1, 0,
                        static_cast<int>(mContent_array.size() - 1), "%d", ImGuiSliderFlags_AlwaysClamp);

                    if (check_index != current_index) { 
                        strcpy_s(edit_buffer, sizeof(edit_buffer), mContent_array[current_index].c_str());
                        check_index = current_index;
                    }
                    
                    ImGui::Text("Current Content Text");
                    ImGui::InputTextMultiline("##Current Content Text", edit_buffer, sizeof(edit_buffer), ImVec2(0, 80));
                    if (ImGui::Button("Edit current content")) {
                        mContent_array[current_index] = std::string(edit_buffer);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Remove current content")) {
                        mContent_array.erase(mContent_array.begin() + current_index);
                        if (current_index > 0) --current_index;
                    }

                    ImGui::Text("Current text position");
                    ImGui::Text("X"); ImGui::SameLine();
                    ImGui::DragFloat("##TextPosX", &position_vector[current_index].first, 0.01f);
                    ImGui::Text("Y"); ImGui::SameLine();
                    ImGui::DragFloat("##TextPosY", &position_vector[current_index].second, 0.01f);
                }
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
                        changed = true;
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
            
            ImGui::Text("Rotation"); ImGui::SameLine();
            changed |= ImGui::DragFloat("##TextRot", &Rot(), 5.f, 0.f, 360.f);
            ImGui::Text("Line Height"); ImGui::SameLine();
            ImGui::DragInt("##Line Height", &Get_line_height(), 1);
            ImGui::Text("Layering Order"); ImGui::SameLine();
            ImGui::DragInt("##Layering Order", &LayerOrder, 1, 0, 9);
            ImGui::Checkbox("Is Visible", &visibility);
        }
        return changed;
    }

    /*!*************************************************************************
    Deserialize for Text Component
    ****************************************************************************/
    bool Text::Deserialize(const rapidjson::Value& obj)
    {
        if (obj.HasMember("text type") && obj["text type"].IsInt())
            TEXT_type = obj["text type"].GetInt();

        if (obj.HasMember("Content") && obj["Content"].IsString())
            mContent = obj["Content"].GetString();

        if (obj.HasMember("Array of Content") && obj["Array of Content"].IsArray())
        {
            const auto& arr = obj["Array of Content"];
            for (const auto& arr_data : arr.GetArray())
            {
                if (arr_data.IsString()) mContent_array.push_back(arr_data.GetString());
            }
        }

        if (obj.HasMember("Vector of text position") && obj["Vector of text position"].IsArray())
        {
            const auto& arr = obj["Vector of text position"];
            for (const auto& arr_data : arr.GetArray())
            {
                if (arr_data.IsArray()) {
                    position_vector.push_back({ arr_data[0].GetFloat(),arr_data[1].GetFloat() });
                }
            }
        }

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

        if (obj.HasMember("LayeringOrder") && obj["LayeringOrder"].IsInt())
            LayerOrder = obj["LayeringOrder"].GetInt();

        if (obj.HasMember("Visibility") && obj["Visibility"].IsBool())
            visibility = obj["Visibility"].GetBool();

        return true;
    }

    /*!*************************************************************************
    Serialize for Text Component
    ****************************************************************************/
    bool Text::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
        writer->Key("text type");
        writer->Int(TEXT_type);

        writer->Key("Content");
        writer->String(mContent.c_str());

        writer->Key("Array of Content");
        writer->StartArray(); //start array here
        for (const auto& content : mContent_array) {
            writer->String(content.c_str()); //serialize array of data here
        }
        writer->EndArray(); //end array here

        writer->Key("Vector of text position");
        writer->StartArray(); //start array here
        for (const auto& pos : position_vector) {
            writer->StartArray(); //start array here
            writer->Double(pos.first); //serialize array of data here
            writer->Double(pos.second); //serialize array of data here
            writer->EndArray(); //end array here
        }
        writer->EndArray(); //end array here

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

        writer->Key("LayeringOrder");
        writer->Int(LayerOrder);

        writer->Key("Visibility");
        writer->Bool(visibility);

        return true;
    }
}
