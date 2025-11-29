/*!------------------------------------------------------------------------------------------------
	File: ECSComponentParsing.cpp
	Project: NocturnalStudio

		This file contains the declarations oif functions FromJson for various ECS Components, which
		allows ECS components to be parsed from a JSON file

	Primary Author:
		Laycock Enhui Sarah (95%)
		- Created and implemented 

	Rishika added AIComponent

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS/ECS_Component.h"
#include "System/Audio_System.h"
#include "ECS/ECS.h"


	//  Texture lookup hook for sprite_path, GL texture id 
	static std::function<int(const std::string&)> sTextureLokup;

	// Safe fallback that compiles even if nobody sets sTextureLookup.
	static int ResolveTextureIDPser(const char* p) {
		if (!p || !*p) return 0;
		if (sTextureLokup) return sTextureLokup(std::string{ p });
		return 0; // not wired yet -> invisible sprite (isVisible=false)
	}

	
extern AudioSystem* gAudioSystem;

bool ComponentFactory::CreateFromName(EntityID entity, const std::string& name, const Json& j) {
	ECS& ecs = ECS::getECS();
	if (name=="Transform")
	{
		ecs.addComponent(entity, std::any_cast<Transform>(Create(name,j)));
		return true;
	}
	if (name == "ImageRenderer")
	{
		ecs.addComponent(entity, std::any_cast<ImageRenderer>(Create(name,j)));
		return true;
	}
	if (name == "CircleRenderer")
	{
		ecs.addComponent(entity, std::any_cast<CircleRenderer>(Create(name,j)));
		return true;
	}
	if (name == "RectRenderer")
	{
		ecs.addComponent(entity, std::any_cast<RectRenderer>(Create(name,j)));
		return true;
	}
	if (name == "AnimalBehavior")
	{
		ecs.addComponent(entity, std::any_cast<AnimalBehavior>(Create(name,j)));
		return true;
	}
	if (name == "PhysicsComponent")
	{
		ecs.addComponent(entity, std::any_cast<PhysicsComponent>(Create(name,j)));
		return true;
	}
	if (name == "BoxCollider")
	{
		ecs.addComponent(entity, std::any_cast<BoxCollider>(Create(name,j)));
		return true;
	}
	if (name == "AABBCollider")
	{
		ecs.addComponent<AABBCollider>(entity, std::any_cast<AABBCollider>(Create(name,j)));
		return true;
	}
	if (name=="AIComponent")
	{
		ecs.addComponent<AIComponent>(entity, std::any_cast<AIComponent>(Create(name, j)));
		return true;
	}

	if (name == "AudioData") {
		auto comp = std::any_cast<AudioData>(Create(name, j));

		if (gAudioSystem) {
			gAudioSystem->registerNewAudioResource(comp, entity);
		}

		ecs.addComponent(entity, std::move(comp));
		return true;
	}

	return false;
}

void ComponentFactory::ParseFromName(EntityID entity, const std::string& name, const Json& j) {
	auto it = ParserMap().find(name);

	if (it == ParserMap().end()) return;
	else it->second(entity, j);
}
void Collider::FromJSON(const rapidjson::Value& json){}
void Collider::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {}

void RenderComponent::FromJSON(const rapidjson::Value& json) {
	const char* members[]{ "opacity","is_visible","color_tint","color_to_add","width","height","angle"};
	for (int i{};i<7;i++)
	{
		if (json.HasMember(members[i]))
		{
			switch (i)
			{
			case 0://opacity
				if (json[members[i]].IsFloat())
				{
					opacity = json[members[i]].GetFloat();
				}
				break; 
			case 1://isVisible
				if (json[members[i]].IsBool())
				{
					isVisible = json[members[i]].GetBool();
				}
				break;
			case 2://color_tint
				if (json[members[i]].IsArray())
				{
					size_t limit = json[members[i]].Size() <= 4 ? json[members[i]].Size() : 4;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							colorTint[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if(json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("r") && json[members[i]]["r"].IsFloat())colorTint.x() = json[members[i]]["r"].GetFloat();
					if (json[members[i]].HasMember("g") && json[members[i]]["g"].IsFloat())colorTint.y() = json[members[i]]["g"].GetFloat();
					if (json[members[i]].HasMember("b") && json[members[i]]["b"].IsFloat())colorTint.z() = json[members[i]]["b"].GetFloat();
					if (json[members[i]].HasMember("a") && json[members[i]]["a"].IsFloat())colorTint.a() = json[members[i]]["a"].GetFloat();
				}
				break;
			case 3://color_to_add
				if (json[members[i]].IsArray())
				{
					size_t limit = json[members[i]].Size() <= 4 ? json[members[i]].Size() : 4;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							colorToAdd[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("r") && json[members[i]]["r"].IsFloat())colorToAdd.x() = json[members[i]]["r"].GetFloat();
					if (json[members[i]].HasMember("g") && json[members[i]]["g"].IsFloat())colorToAdd.y() = json[members[i]]["g"].GetFloat();
					if (json[members[i]].HasMember("b") && json[members[i]]["b"].IsFloat())colorToAdd.z() = json[members[i]]["b"].GetFloat();
					if (json[members[i]].HasMember("a") && json[members[i]]["a"].IsFloat())colorToAdd.a() = json[members[i]]["a"].GetFloat();
				}
				break;
			case 4://width
				if (json[members[i]].IsFloat())
				{
					width = json[members[i]].GetFloat();
				}
				break;
			case 5://height
				if (json[members[i]].IsFloat())
				{
					height = json[members[i]].GetFloat();
				}
				break;
			}
		}
	}
	
}

void RenderComponent::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {

	if(opacity<=1.f)json.AddMember("opacity", opacity, allocator);

	if (!isVisible)json.AddMember("is_visible", isVisible, allocator);

	if (colorTint!=Vector4D{1.0f, 1.0f, 1.0f, 1.0f})
	{
		rapidjson::Value tint(rapidjson::kObjectType);
		tint.AddMember("r", colorTint.x(), allocator);
		tint.AddMember("g", colorTint.y(), allocator);
		tint.AddMember("b", colorTint.z(), allocator);
		tint.AddMember("a", colorTint.a(), allocator);
		json.AddMember("color_tint", tint, allocator);
	}


	if (colorTint != Vector4D{ 0.0f, 0.0f, 0.0f, 0.0f })
	{
		rapidjson::Value addc(rapidjson::kObjectType);
		addc.AddMember("r", colorToAdd.x(), allocator);
		addc.AddMember("g", colorToAdd.y(), allocator);
		addc.AddMember("b", colorToAdd.z(), allocator);
		addc.AddMember("a", colorToAdd.a(), allocator);
		json.AddMember("color_to_add", addc, allocator);
	}

}

void Transform::FromJSON(const rapidjson::Value& json) {
	const char* members[]{ 
		"position",		//0
		"scale",		//1
		"rotation",		//2
		"height",		//3
		"space",		//4
		"is_visible",	//5
		"rotatable",	//6
		"scalable"
	};

	//if any of these values is missing, its fine. checks for specific values first (HasMember())
	//then it will also check for each param type
	for (int i{}; i < 7; i++)
	{
		if (json.HasMember(members[i]))
		{
			switch (i)
			{
			case 0://position
				if (json[members[i]].IsArray())
				{
					size_t limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							position[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("x") && json[members[i]]["x"].IsFloat())position.x() = json[members[i]]["x"].GetFloat();
					if (json[members[i]].HasMember("y") && json[members[i]]["y"].IsFloat())position.y() = json[members[i]]["y"].GetFloat();
				}
				else if (json[members[i]].IsString() && json[members[i]].GetString() == "random")
				{
					randomPos = true; 
				}
				break;
			case 1://scale
				if (json[members[i]].IsArray())
				{
					size_t limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							scale[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("x") && json[members[i]]["x"].IsFloat())scale.x() = json[members[i]]["x"].GetFloat();
					if (json[members[i]].HasMember("y") && json[members[i]]["y"].IsFloat())scale.y() = json[members[i]]["y"].GetFloat();
				}
				else if (json[members[i]].IsFloat())
				{
					scale = { json[members[i]].GetFloat(),json[members[i]].GetFloat() };
				}
				break;
			case 2://rotation
				if (json[members[i]].IsFloat())
				{
					rotation = json[members[i]].GetFloat();
				}
				else if (json[members[i]].IsString()&& json[members[i]].GetString()=="random")
				{
					randomRot = true;
				}
				break;
			case 3://height
				if (json[members[i]].IsFloat())
				{
					height = json[members[i]].GetFloat();
				}
				break;
			case 4://space
				if (json[members[i]].IsString())
				{

					std::string idk{ json[members[i]].GetString() };
					if (idk=="world"||idk=="World"||idk=="w"||idk=="W")
					{
						space = CoordinateSpace::World;
					}
					else if (idk == "Viewport" || idk == "viewport" || idk == "v" || idk == "V") {
						space = CoordinateSpace::Viewport;
					}
				}
				break;
			case 5:
				if (json[members[i]].IsBool())
				{
					isVisible = json[members[i]].GetBool();
				}
				break;
			case 6://isrotatable
				if (json[members[i]].IsBool())
				{
					isRotatable = json[members[i]].GetBool();
				}
				break;
			case 7://isScalable
				if (json[members[i]].IsBool())
				{
					isScalable = json[members[i]].GetBool();
				}
			}
		}
	}
}

void Transform::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	

	json.SetObject();
	rapidjson::Value typeVal;
	typeVal.SetString("Transform", allocator);
	json.AddMember("type", typeVal, allocator);

	rapidjson::Value posobj(rapidjson::kObjectType);
	posobj.AddMember("x", position.x(), allocator);
	posobj.AddMember("y", position.y(), allocator);
	json.AddMember("position", posobj, allocator);

	rapidjson::Value scal(rapidjson::kObjectType);
	scal.AddMember("x", scale.x(), allocator);
	scal.AddMember("y", scale.y(), allocator);
	json.AddMember("scale", scal, allocator);

	json.AddMember("rotation", rotation, allocator);
	
	json.AddMember("height", height, allocator);

	rapidjson::Value spaceVal;
	if (space == CoordinateSpace::Viewport) spaceVal.SetString("viewport", allocator);
	else spaceVal.SetString("world", allocator);
	json.AddMember("space", spaceVal, allocator);

	json.AddMember("is_visible", isVisible, allocator);

	json.AddMember("rotatable", isRotatable, allocator);

	json.AddMember("scalable", isScalable, allocator);

}

void ImageRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	const char* members[]{
		"texture_id",//0
		"image_path",//1
		"sprite_path",
		
	};
	for (int i{}; i < 3; i++)
	{
		if (json.HasMember(members[i]))
		{
			switch (i)
			{
			case 0://tex id
				if (json[members[i]].IsInt())
				{
					textureID = json[members[i]].GetInt();
				}
				break;
			case 1://isVisible
				if (json[members[i]].IsString())
				{
					image_path = json[members[i]].GetString();
					textureID = ResolveTextureIDPser(json[members[i]].GetString());
					isVisible = (textureID > 0);
				}
				break;
			case 2://isVisible
				if (json[members[i]].IsString())
				{
					image_path = json[members[i]].GetString();
					textureID = ResolveTextureIDPser(json[members[i]].GetString());
					isVisible = (textureID > 0);
				}
				break;
			}
		}
	}
}

void ImageRenderer::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("ImageRenderer", allocator);
	json.AddMember("type", typeVal, allocator);

	RenderComponent::ToJSON(json, allocator);

	rapidjson::Value spriteVal;
	spriteVal.SetString(image_path.c_str(), static_cast<rapidjson::SizeType>(image_path.size()), allocator);
	json.AddMember("sprite_path", spriteVal, allocator);
}



void CircleRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	if (json.HasMember("radius") && json["radius"].IsFloat())
	{

		radius = json["radius"].GetFloat();

	}
}

void CircleRenderer::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();
	rapidjson::Value typeVal;
	typeVal.SetString("CircleRenderer", allocator);
	json.AddMember("type", typeVal, allocator);

	RenderComponent::ToJSON(json, allocator);

	json.AddMember("radius", radius, allocator);
}

void RectRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	const char* members[]{
		"width",//0
		"height"//1
	};
	for (int i{}; i < 2; i++)
	{
		if (json.HasMember(members[i])&& json[members[i]].IsFloat())
		{
			switch (i)
			{
			case 0://width
				width = json[members[i]].GetFloat();
				break;
			case 1://height
				height = json[members[i]].GetFloat();
				break;
			}
		}
	}
}

void RectRenderer::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();
	rapidjson::Value typeVal;
	typeVal.SetString("RectRenderer", allocator);
	json.AddMember("type", typeVal, allocator);

	RenderComponent::ToJSON(json, allocator);



	json.AddMember("width", width, allocator);
	json.AddMember("height", height, allocator);
}

void AnimalBehavior::FromJSON(const rapidjson::Value& json) {
	const char* members[]{
		"health_bar",//0
		"hunger_bar",//1
		"sanity_bar",//2
		"slower_decreasing_rate",//3
		"normal_decreasing_rate",//4
		"faster_decreasing_rate",//5
		"penalty_decreasing_rate"//6

	};
	for (int i{}; i < 7; i++)
	{
		if (json.HasMember(members[i])&& json[members[i]].IsDouble())
		{
			double jvalue = json[members[i]].GetDouble();
			switch (i)
			{
			case 0://health bar
				health_bar = jvalue;
				break;
			case 1://hunger
				hunger_bar = jvalue;
				break;
			case 2://sanity
				sanity_bar = jvalue;
				break;
			case 3: //slower dec rate
				slower_decreasing_rate = jvalue;
				break;
			case 4: //normal dec rate
				normal_decreasing_rate = jvalue;
				break;
			case 5: //faster dec rate
				faster_decreasing_rate = jvalue;
				break;
			case 6: //penalty dec rate
				penalty_decreasing_rate = jvalue;
				break;
			}
		}
	}
}

void AnimalBehavior::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();
	rapidjson::Value typeVal;
	typeVal.SetString("AnimalBehavior", allocator);
	json.AddMember("type", typeVal, allocator);



	json.AddMember("health_bar", health_bar, allocator);
	json.AddMember("hunger_bar", hunger_bar, allocator);
	json.AddMember("sanity_bar", sanity_bar, allocator);
}

void PhysicsComponent::FromJSON(const rapidjson::Value& json) {
	const char* members[]{
		"position",//0
		"velocity",//1
		"acceleration",//2
		"gravity_enabled"//3
	};
	size_t limit{};
	for (int i{}; i < 3; i++)
	{
		if (json.HasMember(members[i]))
		{
			switch (i)
			{
			case 0://position
				if (json[members[i]].IsArray())
				{
					limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							position[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("x") && json[members[i]]["x"].IsFloat())position.x() = json[members[i]]["x"].GetFloat();
					if (json[members[i]].HasMember("y") && json[members[i]]["y"].IsFloat())position.y() = json[members[i]]["y"].GetFloat();
				}
				
				break;
			case 1://velocity
				if (json[members[i]].IsArray())
				{
					limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							velocity[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("x") && json[members[i]]["x"].IsFloat())velocity.x() = json[members[i]]["x"].GetFloat();
					if (json[members[i]].HasMember("y") && json[members[i]]["y"].IsFloat())velocity.y() = json[members[i]]["y"].GetFloat();
				}
				break;
			case 2://acceleration
				if (json[members[i]].IsArray())
				{
					limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
					for (size_t j = 0; j < limit; j++)
					{
						if (json[members[i]][j].IsFloat())
						{
							acceleration[j] = json[members[i]][j].GetFloat();
						}
					}
				}
				else if (json[members[i]].IsObject()) {
					if (json[members[i]].HasMember("x") && json[members[i]]["x"].IsFloat())acceleration.x() = json[members[i]]["x"].GetFloat();
					if (json[members[i]].HasMember("y") && json[members[i]]["y"].IsFloat())acceleration.y() = json[members[i]]["y"].GetFloat();
				}

				break;
			case 3:
				if (json[members[i]].IsBool())
				{
					gravityEnabled = json[members[i]].GetBool();
				}
				break;
			}
		}
	}
}

void PhysicsComponent::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("PhysicsComponent", allocator);
	json.AddMember("type", typeVal, allocator);


	rapidjson::Value posobj(rapidjson::kObjectType);
	posobj.AddMember("x", position.x(),allocator);
	posobj.AddMember("y", position.y(), allocator);
	json.AddMember("position", posobj, allocator);
	
	rapidjson::Value velo(rapidjson::kObjectType);
	velo.AddMember("x", velocity.x(), allocator);
	velo.AddMember("y", velocity.y(), allocator);
	json.AddMember("velocity", velo, allocator);

	rapidjson::Value acce(rapidjson::kObjectType);
	acce.AddMember("x", acceleration.x(), allocator);
	acce.AddMember("y", acceleration.y(), allocator);
	json.AddMember("acceleration", acce, allocator);

	json.AddMember("gravity_enabled", gravityEnabled, allocator);
	
}

void BoxCollider::FromJSON(const rapidjson::Value& json) {
	const char* members[]{
		"width",//0
		"height"//1
	};
	for (int i{}; i < 2; i++)
	{
		if (json.HasMember(members[i]) && json[members[i]].IsFloat())
		{
			switch (i)
			{
			case 0://width
				width = json[members[i]].GetFloat();
				break;
			case 1://height
				height = json[members[i]].GetFloat();
				break;
			}
		}
	}
	if (json.HasMember("bounding_box_size") && json["bounding_box_size"].IsFloat())
	{
		width = height = json["bounding_box_size"].GetFloat();
	}
}

void BoxCollider::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("BoxCollider", allocator);
	json.AddMember("type", typeVal, allocator);

	json.AddMember("width", width, allocator);
	json.AddMember("height", height, allocator);

}

void AIComponent::FromJSON(const rapidjson::Value& component_json) {
	

		if (component_json.HasMember("state") && component_json["state"].IsString()) {
			std::string s = component_json["state"].GetString();
			if (s == "Chase") state = AIComponent::State::Chase;
			else if (s == "Flee")  state = AIComponent::State::Flee;
			else                   state = AIComponent::State::Wander;
		}
		if (component_json.HasMember("maxSpeed") && component_json["maxSpeed"].IsNumber())      maxSpeed = component_json["maxSpeed"].GetFloat();
		if (component_json.HasMember("accel") && component_json["accel"].IsNumber())         accel = component_json["accel"].GetFloat();
		if (component_json.HasMember("wanderJitter") && component_json["wanderJitter"].IsNumber())  wanderJitter = component_json["wanderJitter"].GetFloat();
		if (component_json.HasMember("chaseStrength") && component_json["chaseStrength"].IsNumber()) chaseStrength = component_json["chaseStrength"].GetFloat();
		if (component_json.HasMember("fleeStrength") && component_json["fleeStrength"].IsNumber())  fleeStrength = component_json["fleeStrength"].GetFloat();
		if (component_json.HasMember("avoidRadius") && component_json["avoidRadius"].IsNumber())   avoidRadius = component_json["avoidRadius"].GetFloat();
		if (component_json.HasMember("targetName") && component_json["targetName"].IsString())
			targetName = component_json["targetName"].GetString();
		if (component_json.HasMember("targetWeight") && component_json["targetWeight"].IsNumber())
			targetWeight = component_json["targetWeight"].GetFloat();
}

void AIComponent::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("AIComponent", allocator);
	json.AddMember("type", typeVal, allocator);
}

void AABBCollider::FromJSON(const rapidjson::Value& json) {
	if (json.HasMember("bounding_box_size") && json["bounding_box_size"].IsFloat())
	{
		width = height = json["bounding_box_size"].GetFloat();
	}
}

void AABBCollider::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const {
	json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("AABBCollider", allocator);
	json.AddMember("type", typeVal, allocator);

	json.AddMember("bounding_box_size", width, allocator);
}

static AudioData::IndividualAudioData::Group AudioGroupFromString(const std::string& str) {
	std::string v = str;
	std::transform(v.begin(), v.end(), v.begin(), ::tolower);

	if (v == "music") return AudioData::IndividualAudioData::Group::Music;
	if (v == "sfx") return AudioData::IndividualAudioData::Group::SFX;
	if (v == "reserved1") return AudioData::IndividualAudioData::Group::RESERVED1;
	if (v == "reserved2") return AudioData::IndividualAudioData::Group::RESERVED2;

	return AudioData::IndividualAudioData::Group::SFX;
}

static const char* AudioGroupToString(AudioData::IndividualAudioData::Group group) {
	switch (group) {
		case AudioData::IndividualAudioData::Group::Music: return "Music";
		case AudioData::IndividualAudioData::Group::SFX: return "SFX";
		case AudioData::IndividualAudioData::Group::RESERVED1: return "RESERVED1";
		case AudioData::IndividualAudioData::Group::RESERVED2: return "RESERVED2";
		default: return "SFX";
	}
}

void AudioData::FromJSON(const rapidjson::Value& json) {

    audios.clear();
    listener = false;

    if (json.HasMember("listener") && json["listener"].IsBool()) {
        listener = json["listener"].GetBool();
    }

    if (!json.HasMember("audios") || !json["audios"].IsArray()) {
        return;
    }

    const auto& arr = json["audios"];
    for (auto it = arr.Begin(); it != arr.End(); ++it) {
        if (!it->IsObject()) continue;
        const rapidjson::Value& o = *it;

        AudioData::IndividualAudioData a{};

        if (o.HasMember("resource_id") && o["resource_id"].IsUint()) {
            a.resourceID = o["resource_id"].GetUint();
        } else {
            a.resourceID = 0; // unresolved; will resolve by name in AudioSystem
        }

        if (o.HasMember("name") && o["name"].IsString())
            a.name = o["name"].GetString();

        if (o.HasMember("loop") && o["loop"].IsBool()) a.loop = o["loop"].GetBool();
        if (o.HasMember("to_play_at_start") && o["to_play_at_start"].IsBool()) a.toPlayAtStart = o["to_play_at_start"].GetBool();
        if (o.HasMember("locality") && o["locality"].IsBool()) a.locality = o["locality"].GetBool();

        if (o.HasMember("persist_on_entity_destruction") && o["persist_on_entity_destruction"].IsBool())
            a.persistOnEntityDestruction = o["persist_on_entity_destruction"].GetBool();
        if (o.HasMember("persist_on_scene_change") && o["persist_on_scene_change"].IsBool())
            a.persistOnSceneChange = o["persist_on_scene_change"].GetBool();

        if (o.HasMember("volume") && o["volume"].IsNumber())
            a.volume = std::clamp(static_cast<float>(o["volume"].GetDouble()), 0.0f, 1.0f);
        if (o.HasMember("pitch") && o["pitch"].IsNumber())
            a.pitch = static_cast<float>(o["pitch"].GetDouble());
        if (o.HasMember("distance_mult") && o["distance_mult"].IsNumber())
            a.distanceMult = static_cast<float>(o["distance_mult"].GetDouble());

        if (o.HasMember("group") && o["group"].IsString())
            a.group = AudioGroupFromString(o["group"].GetString());

        audios.push_back(std::move(a));
    }
}

void AudioData::ToJSON(rapidjson::Value& json, rapidjson::Document::AllocatorType& alloc) const {
    json.SetObject();

	rapidjson::Value typeVal;
	typeVal.SetString("AudioData", alloc);
	json.AddMember("type", typeVal, alloc);
    rapidjson::Value aud(rapidjson::kArrayType);
    for (const auto& a : audios) {
        rapidjson::Value item(rapidjson::kObjectType);

        item.AddMember("resource_id", a.resourceID, alloc);

        rapidjson::Value nameV;
        nameV.SetString(a.name.c_str(), static_cast<rapidjson::SizeType>(a.name.size()), alloc);
        item.AddMember("name", nameV, alloc);

        item.AddMember("loop", a.loop, alloc);
        item.AddMember("to_play_at_start", a.toPlayAtStart, alloc);
        item.AddMember("locality", a.locality, alloc);
        item.AddMember("persist_on_entity_destruction", a.persistOnEntityDestruction, alloc);
        item.AddMember("persist_on_scene_change", a.persistOnSceneChange, alloc);

        item.AddMember("volume", a.volume, alloc);
        item.AddMember("pitch", a.pitch, alloc);
        item.AddMember("distance_mult", a.distanceMult, alloc);

        const char* grp = AudioGroupToString(a.group);
        rapidjson::Value grpV;
        grpV.SetString(grp, static_cast<rapidjson::SizeType>(std::strlen(grp)), alloc);
        item.AddMember("group", grpV, alloc);

        aud.PushBack(item, alloc);
    }

    json.AddMember("audios", aud, alloc);
    json.AddMember("listener", listener, alloc);
}

