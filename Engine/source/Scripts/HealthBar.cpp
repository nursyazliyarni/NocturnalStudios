/*!------------------------------------------------------------------------------------------------
	File: HealthBar.cpp
	Project: NocturnalStudio

	Brief:
		The HealthBar system manages the displaying of Various Healthbars of Anumals Bars of an 

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the HealthBar class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/HealthBar.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/HUDComponent.h"
#include "ECS/Components/AnimalAttributes.h"


namespace NocturnalEngine {

	void HealthBar::Start()
	{

		//this saves a copy of the origin at generation.
		origin = p_ecs.GetComponent<Transform>(entityID);
		//this runs whenever the scene is started. 
		//This loop is going through all the entities and finding the animal 
		// (the first entity with AnimalAttributes component)
		for (Entity e : p_ecs.GetAliveEntitiesSet())
		{
			if (p_ecs.HaveComponent<AnimalAttributes>(e) )
			{
				
				linkedAnimal = e;
				//std::cout << "animal found for entity"<< p_ecs.GetComponent<NameTag>(entityID).GetNameTag()<<"\n";
				
			}

		}
	}

	void HealthBar::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);

		//check if the connected entity has HUD component 
		//(by right it should, just in case)
		if (!p_ecs.HaveComponent<HUDComponent>(entityID)) return;

		HUDComponent& hudC{ p_ecs.GetComponent<HUDComponent>(entityID) };

		//make sure the animal exists lol
		if (linkedAnimal == 0 || !p_ecs.HaveComponent<AnimalAttributes>(linkedAnimal))return;
		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(linkedAnimal) };

		float percentageHP{};
		switch (hudC.GetType())
		{
		case HUDComponent::ElementType::HealthBar:
			percentageHP = static_cast<float>(aA.current_health / aA.maxHealth);
			break;
		case HUDComponent::ElementType::HungerBar:
			percentageHP = static_cast<float>(aA.current_hunger / aA.maxHunger);
			break;
		case HUDComponent::ElementType::SanityBar:
			percentageHP = static_cast<float>(aA.current_sanity / aA.maxSanity);
			break;
		}

		float currrentPercentageScale{ origin.GetScale().x * percentageHP };

		p_ecs.GetComponent<Transform>(entityID).GetScale().x = currrentPercentageScale;
		p_ecs.GetComponent<Transform>(entityID).SetPosFromCenterLeft(origin.GetMinPos().x, origin.GetPos().y);

	}

	void HealthBar::End()
	{
		//this resets the origin when reloading a scene
		p_ecs.GetComponent<Transform>(entityID) = origin;
	}

	
	

}
