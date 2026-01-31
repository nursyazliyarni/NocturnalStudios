/*!------------------------------------------------------------------------------------------------
	File: HealthBar.cpp
	Project: NocturnalStudio

	Brief:
		The HealthBar system manages the displaying of Various Healthbars of Anumals Bars of an 

	Primary Author:
		 ANG YU XUAN (100%)
		- Created the HealthBar class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/HealthBar.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/HUDComponent.h"
#include "ECS/Components/AnimalAttributes.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	void HealthBar::Start()
	{

		//this saves a copy of the origin at generation.
		//This is important as we need the original size of the 
		//full hp bar when calculating how wide we need it
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
		//std::cout << "Entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " script running\n";
		//check if the connected entity has HUD component 
		//(by right it should, just in case)
		if (!p_ecs.HaveComponent<HUDComponent>(entityID)) return;

		//gets the GUD component from the entity this script is connected to
		HUDComponent& hudC{ p_ecs.GetComponent<HUDComponent>(entityID) };

		
		if (linkedAnimal == 0)std::cout << "Entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " cannot find an animal!\n"; 
		if (!p_ecs.HaveComponent<AnimalAttributes>(linkedAnimal))std::cout << "Entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << "'s animal has no Animal Attributes!\n";
		//make sure the animal exists lol
		if (linkedAnimal == 0 || !p_ecs.HaveComponent<AnimalAttributes>(linkedAnimal))return;
		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(linkedAnimal) };

		//check which type of hud this is, 
		// this will then dictate what kind of info it takes from AnimalAttributes
		float percentageHP{};
		switch (hudC.GetType())
		{//the percentage is calculated based on the current health divided by the max health
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

		//uses the original scale of the entity and multiplies 
		//it based on the percentage of hp calculated above (this determines how wide it is)
		float currrentPercentageScale{ origin.GetScale().x * percentageHP };

		//if animal is dead, do not show any of the bars
		//this is not shown in dev mode for convenience
#if !DEBUG
		if (!aA.is_alive)currrentPercentageScale = 0.f;
#endif
		//sets the scale, basically how wide the bar will render
		p_ecs.GetComponent<Transform>(entityID).GetScale().x = currrentPercentageScale;
		//sets the origin, 
		// since the ecs right now can only render from middle center 
		// i added a helper function that sets the position of where we want the extreme left
		//side of the bar is
		p_ecs.GetComponent<Transform>(entityID).SetPosFromCenterLeft(origin.GetMinPos().x, origin.GetPos().y);

		

	}

	void HealthBar::End()
	{
		//this resets the origin when reloading a scene
		p_ecs.GetComponent<Transform>(entityID) = origin;
	}

	
	

}
