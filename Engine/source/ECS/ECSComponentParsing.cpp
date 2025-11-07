#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"
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
	if (name == "SpriteRenderer")
	{
		ecs.addComponent(entity, std::any_cast<SpriteRenderer>(Create(name,j)));
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
	if (name == "AudioData")
	{
		ecs.addComponent(entity, std::any_cast<AudioData>(Create(name,j)));
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
		return false;
	}
}
void Collider::FromJSON(const rapidjson::Value& json){}

void RenderComponent::FromJSON(const rapidjson::Value& json) {
	const char* members[]{ "opacity","is_visible","color_tint","color_to_add"};
	for (int i{};i<4;i++)
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
				break;
			}
		}
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

void ImageRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	const char* members[]{
		"texture_id",//0
		"image_path"//1
	};
	for (int i{}; i < 2; i++)
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
				}
				break;
			}
		}
	}
}

void SpriteRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	if (json.HasMember("sprite_path")&& json["sprite_path"].IsString())
	{
		
			spritePath = json["sprite_path"].GetString();
		
	}
}

void CircleRenderer::FromJSON(const rapidjson::Value& json) {
	RenderComponent::FromJSON(json);
	if (json.HasMember("radius") && json["radius"].IsFloat())
	{

		radius = json["radius"].GetFloat();

	}
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

void AnimalBehavior::FromJSON(const rapidjson::Value& json) {
	const char* members[]{
		"health_bar",//0
		"hunger_bar",//1
		"sanity_bar"//2
	};
	for (int i{}; i < 3; i++)
	{
		if (json.HasMember(members[i])&& json[members[i]].IsDouble())
		{
			switch (i)
			{
			case 0://health bar
				health_bar = json[members[i]].GetDouble();
				break;
			case 1://hunger
				hunger_bar = json[members[i]].GetDouble();
				break;
			case 2://sanity
				sanity_bar = json[members[i]].GetDouble();
				break;
			}
		}
	}
}

void AudioData::FromJSON(const rapidjson::Value& json) {
	//bro

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
		if (json.HasMember(members[i]) && json[members[i]].IsDouble())
		{
			switch (i)
			{
			case 0://position
				limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
				for (size_t j = 0; j < limit; j++)
				{
					if (json[members[i]][j].IsFloat())
					{
						position[j] = json[members[i]][j].GetFloat();
					}
				}
				break;
			case 1://velocity
				limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
				for (size_t j = 0; j < limit; j++)
				{
					if (json[members[i]][j].IsFloat())
					{
						velocity[j] = json[members[i]][j].GetFloat();
					}
				}
				break;
			case 2://acceleration
				limit = json[members[i]].Size() <= 2 ? json[members[i]].Size() : 2;
				for (size_t j = 0; j < limit; j++)
				{
					if (json[members[i]][j].IsFloat())
					{
						acceleration[j] = json[members[i]][j].GetFloat();
					}
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
}

void AABBCollider::FromJSON(const rapidjson::Value& json) {
	if (json.HasMember("bounding_box_size") && json["bounding_box_size"].IsFloat())
	{
		width = height = json["bounding_box_size"].GetFloat();
	}
}
