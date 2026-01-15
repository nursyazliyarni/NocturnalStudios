/*!------------------------------------------------------------------------------------------------
    File: AnimalAttributes.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize animal attribute components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the animal attributes class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/AnimalAttributes.h"

NocturnalEngine::AnimalAttributes::AnimalAttributes() : health(0.0), maxHealth(0.0),
	sanity(0.0), maxSanity(0.0),
	hunger(0.0), maxHunger(0.0),
	healthDecayRate(0.0), sanityDecayRate(0.0), hungerDecayRate(0.0) {}

bool NocturnalEngine::AnimalAttributes::Deserialize(const rapidjson::Value& obj)
{
	health = obj["Health"].GetDouble();
	maxHealth = obj["Max Health"].GetDouble();
	sanity = obj["Sanity"].GetDouble();
	maxSanity = obj["Max Sanity"].GetDouble();
	hunger = obj["Hunger"].GetDouble();
	maxHunger = obj["Max Hunger"].GetDouble();
	healthDecayRate = obj["Health Decay Rate"].GetDouble();
	sanityDecayRate = obj["Sanity Decay Rate"].GetDouble();
	hungerDecayRate = obj["Hunger Decay Rate"].GetDouble();
	return true;
}

bool NocturnalEngine::AnimalAttributes::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
	writer->Key("Health");
	writer->Double(health);
	writer->Key("Max Health");
	writer->Double(maxHealth);
	writer->Key("Sanity");
	writer->Double(sanity);
	writer->Key("Max Sanity");
	writer->Double(maxSanity);
	writer->Key("Hunger");
	writer->Double(hunger);
	writer->Key("Max Hunger");
	writer->Double(maxHunger);
	writer->Key("Health Decay Rate");
	writer->Double(healthDecayRate);
	writer->Key("Sanity Decay Rate");
	writer->Double(sanityDecayRate);
	writer->Key("Hunger Decay Rate");
	writer->Double(hungerDecayRate);
	return true;
}