#include "ECS/ECS_Component.h"

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

void ImageRenderer::FromJSON(const rapidjson::Value& json) {}

void SpriteRenderer::FromJSON(const rapidjson::Value& json) {}

void CircleRenderer::FromJSON(const rapidjson::Value& json) {}

void RectRenderer::FromJSON(const rapidjson::Value& json) {}

void AnimalBehavior::FromJSON(const rapidjson::Value& json) {}

void AudioData::FromJSON(const rapidjson::Value& json) {}

void PhysicsComponent::FromJSON(const rapidjson::Value& json) {}

void BoxCollider::FromJSON(const rapidjson::Value& json) {}

void AABBCollider::FromJSON(const rapidjson::Value& json) {}