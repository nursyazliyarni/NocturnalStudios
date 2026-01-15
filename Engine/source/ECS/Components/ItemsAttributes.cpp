#include "pch.h"
#include "ECS/Components/ItemsAttributes.h"

NocturnalEngine::ItemsAttributes::ItemsAttributes() : healthRestore(0.0), healthRegenRate(0.0), maxHealthRestore(0.0),
	sanityRestore(0.0), sanityRegenRate(0.0), maxSanityRestore(0.0),
	hungerRestore(0.0), hungerRegenRate(0.0), maxHungerRestore(0.0) {}

bool NocturnalEngine::ItemsAttributes::Deserialize(const rapidjson::Value& obj)
{
	healthRestore = obj["Health Restore"].GetDouble();
	healthRegenRate = obj["Health Regen Rate"].GetDouble();
	maxHealthRestore = obj["Max Health Restore"].GetDouble();
	sanityRestore = obj["Sanity Restore"].GetDouble();
	sanityRegenRate = obj["Sanity Regen Rate"].GetDouble();
	maxSanityRestore = obj["Max Sanity Restore"].GetDouble();
	hungerRestore = obj["Hunger Restore"].GetDouble();
	hungerRegenRate = obj["Hunger Regen Rate"].GetDouble();
	maxHungerRestore = obj["Max Hunger Restore"].GetDouble();
	return true;
}

bool NocturnalEngine::ItemsAttributes::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
	writer->Key("Health Restore");
	writer->Double(healthRestore);
	writer->Key("Health Regen Rate");
	writer->Double(healthRegenRate);
	writer->Key("Max Health Restore");
	writer->Double(maxHealthRestore);
	writer->Key("Sanity Restore");
	writer->Double(sanityRestore);
	writer->Key("Sanity Regen Rate");
	writer->Double(sanityRegenRate);
	writer->Key("Max Sanity Restore");
	writer->Double(maxSanityRestore);
	writer->Key("Hunger Restore");
	writer->Double(hungerRestore);
	writer->Key("Hunger Regen Rate");
	writer->Double(hungerRegenRate);
	writer->Key("Max Hunger Restore");
	writer->Double(maxHungerRestore);
	return true;
}
