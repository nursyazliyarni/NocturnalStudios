/*!------------------------------------------------------------------------------------------------
    File: AnimalAttributes.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize animal attribute components for ECS.
	
    Primary Author:
         Ang Yu Xuan (100%)
		- Created the animal attributes class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/AnimalAttributes.h"

NocturnalEngine::AnimalAttributes::AnimalAttributes() : current_health(0.0), maxHealth(0.0),
current_hunger(0.0), maxHunger(0.0), current_sanity(0.0), maxSanity(0.0),healthDecayRate(0.0), 
hungerDecayRate(0.0), sanityDecayRate(0.0) {}

bool NocturnalEngine::AnimalAttributes::Deserialize(const rapidjson::Value& obj)
{
	if (obj.HasMember("Current Health") && obj["Current Health"].IsNumber())current_health = obj["Current Health"].GetDouble();
	if (obj.HasMember("Max Health") && obj["Max Health"].IsNumber())maxHealth = obj["Max Health"].GetDouble();
	if (obj.HasMember("Current Hunger") && obj["Current Hunger"].IsNumber())current_hunger = obj["Current Hunger"].GetDouble();
	if (obj.HasMember("Max Hunger") && obj["Max Hunger"].IsNumber())maxHunger = obj["Max Hunger"].GetDouble();
	if (obj.HasMember("Current Sanity") && obj["Current Sanity"].IsNumber())current_sanity = obj["Current Sanity"].GetDouble();
	if (obj.HasMember("Max Sanity") && obj["Max Sanity"].IsNumber())maxSanity = obj["Max Sanity"].GetDouble();
	if (obj.HasMember("Health Decay Rate") && obj["Health Decay Rate"].IsNumber())healthDecayRate = obj["Health Decay Rate"].GetDouble();
	if (obj.HasMember("Hunger Decay Rate") && obj["Hunger Decay Rate"].IsNumber())hungerDecayRate = obj["Hunger Decay Rate"].GetDouble();
	if (obj.HasMember("Sanity Decay Rate") && obj["Sanity Decay Rate"].IsNumber())sanityDecayRate = obj["Sanity Decay Rate"].GetDouble();
	if (obj.HasMember("IS_alive")&& obj["IS_alive"].IsBool())is_alive = obj["IS_alive"].GetBool();
	return true;
}

bool NocturnalEngine::AnimalAttributes::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
	writer->Key("Current Health");
	writer->Double(current_health);
	writer->Key("Max Health");
	writer->Double(maxHealth);
	writer->Key("Current Hunger");
	writer->Double(current_hunger);
	writer->Key("Max Hunger");
	writer->Double(maxHunger);
	writer->Key("Current Sanity");
	writer->Double(current_sanity);
	writer->Key("Max Sanity");
	writer->Double(maxSanity);
	writer->Key("Health Decay Rate");
	writer->Double(healthDecayRate);
	writer->Key("Hunger Decay Rate");
	writer->Double(hungerDecayRate);
	writer->Key("Sanity Decay Rate");
	writer->Double(sanityDecayRate);
	writer->Key("IS_alive");
	writer->Bool(is_alive);
	return true;
}