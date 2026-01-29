/*!------------------------------------------------------------------------------------------------
	File: DragAndDrop.cpp
	Project: NocturnalStudio

	Brief:
		This file contains the implementation of the DragAndDrop script class which is responsible
		for managing background audio playback within the engine.

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the DragAndDrop class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"

#include "Scripts/DragAndDrop.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Draggable.h"
#include "GUI/GUI.h"

namespace NocturnalEngine {

	void DragAndDrop::Start()
	{
		for (Entity e:p_ecs.GetAliveEntitiesSet())
		{
			
			if (p_ecs.HaveComponent<Draggable>(e)&& p_ecs.GetComponent<Draggable>(e).IsTarget())
			{
				if (p_ecs.HaveComponent<Transform>(e) && p_ecs.HaveComponent<Collider>(e)&& p_ecs.HaveComponent<AnimalAttributes>(e)) {
					dragTarget = e; 
					//std::cout << "animal found for entity"<< p_ecs.GetComponent<NameTag>(entityID).GetNameTag()<<"\n";
				}
			}
			
		}
	}

	void DragAndDrop::Update(float Frametime)
	{
		
		if (!isDragging)
		{
			
			DragCheck();

		}
		else{
			if (!mInput->MouseHold(0)|| mInput->MouseIsReleased(0))
			{
				isDragging = false;
				dupeItem = DestroyDupeItem();
				
			}
			else {
				if (dupeItem==0)
				{
					dupeItem = p_ecs.CreateEntity();
					p_ecs.AddComponent<Transform>(dupeItem, p_ecs.GetComponent<Transform>(entityID));
					p_ecs.AddComponent<Sprite>(dupeItem, p_ecs.GetComponent<Sprite>(entityID));

				}
				if (dupeItem!=0)
				{
					if (p_ecs.HaveComponent<Transform>(dupeItem))
					{
						Transform& t = p_ecs.GetComponent<Transform>(dupeItem);
						t.SetPos(p_GUI->MousePosition.x + dragOffset.x,
							p_GUI->MousePosition.y + dragOffset.y);
					}
					if (CheckAnimalCollision())
					{
						//std::cout << "touching\n";
						isDragging = false;
						dupeItem = DestroyDupeItem();

						AnimalGain();
						
					}
				}
			}
		
		}

	}

	void DragAndDrop::End()
	{

		
	}

	Entity DragAndDrop::DestroyDupeItem() {
		if (dupeItem != 0)
		{
			p_ecs.DestroyEntity(dupeItem);
		}

		return 0;
	}

	void DragAndDrop::DragCheck() {
		if (mInput->MousePressed(0) && p_ecs.HaveComponent<Transform>(entityID)) {
			vec2D mPos{ p_GUI->MousePosition.x,p_GUI->MousePosition.y };
			Transform& t = p_ecs.GetComponent<Transform>(entityID);

			if (p_ecs.HaveComponent<Collider>(entityID))
			{
				Collider& c = p_ecs.GetComponent<Collider>(entityID);
				for (int i = 0; i < 2 && !isDragging; i++)
				{
					vec2D offsetPos{ t.GetPos().x + c[i].mOffset.x ,t.GetPos().y + c[i].mOffset.y };
					switch (c[i].mCol)
					{
					case Collider::ColliderType::circle:
					case Collider::ColliderType::bubble:
						if (distance(mPos, offsetPos) < c[i].mRadius) { isDragging = true; 
						}
						break;
					case Collider::ColliderType::rect:
					case Collider::ColliderType::box:
						vec2D minPt{ offsetPos.x - c[i].mMin.x,offsetPos.y - c[i].mMin.y }, maxPt{ offsetPos.x + c[i].mMax.x,offsetPos.y + c[i].mMax.y };
						if (mPos.x > minPt.x && mPos.x<maxPt.x && mPos.y>minPt.y && mPos.y < maxPt.y)
						{
							isDragging = true;
							//std::cout << "entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " collider check\n";
							
						}
						break;
					}
				}

			}
			else
			{


				if (mPos.x > t.GetMinPos().x && mPos.x < t.GetMaxPos().x &&
					mPos.y>t.GetMinPos().y && mPos.y < t.GetMaxPos().y)
				{
					isDragging = true;
					std::cout << "entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " transform check\n";
					
				}
			}
			if (isDragging)
			{
				dragOffset = { t.GetPos().x - p_GUI->MousePosition.x,t.GetPos().y - p_GUI->MousePosition.y };
				//std::cout << "entity "<< p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " now is dragging\n";
			}
		}
	}
	bool DragAndDrop::CheckAnimalCollision() {

		if (dragTarget!=0)
		{
			
			Transform& dupeT = p_ecs.GetComponent<Transform>(dupeItem);
			Collider& tC= p_ecs.GetComponent<Collider>(dragTarget);
			Transform& tT = p_ecs.GetComponent<Transform>(dragTarget);
			for (int i = 0; i < 2; i++)
			{
				vec2D offsetPos{ tT.GetPos().x + tC[i].mOffset.x ,tT.GetPos().y + tC[i].mOffset.y };
				switch (tC[i].mCol)
				{
				case Collider::ColliderType::circle:
				case Collider::ColliderType::bubble:
					if (distance(dupeT.GetMinPos(), offsetPos) < tC[i].mRadius|| distance(dupeT.GetMaxPos(), offsetPos) < tC[i].mRadius) {
						return true;
					}
					break;
				case Collider::ColliderType::rect:
				case Collider::ColliderType::box:
					vec2D minPt{ offsetPos.x - tC[i].mMin.x,offsetPos.y - tC[i].mMin.y }, maxPt{ offsetPos.x + tC[i].mMax.x,offsetPos.y + tC[i].mMax.y };
					if (dupeT.GetMinPos().x<maxPt.x&&dupeT.GetMaxPos().x>minPt.x&&
						dupeT.GetMinPos().y<maxPt.y && dupeT.GetMaxPos().y>minPt.y)
					{
						//std::cout<<"dupe scale"
						//std::cout << "Animal minX:" << minPt.x << " minY:" << minPt.y << " maxX:" << maxPt.x << " maxY:" << maxPt.y << std::endl;
						//std::cout << "Dupe minX:" << dupeT.GetMinPos().x << " minY:" << dupeT.GetMinPos().y << " maxX:" << dupeT.GetMaxPos().x << " maxY:" << dupeT.GetMaxPos().y << std::endl;
						return true;

					}
					break;
				}
			}
			
			
		}
		return false;
		
	}
	void DragAndDrop::AnimalGain() {
		if (p_ecs.HaveComponent<ItemsAttributes>(entityID))
		{
			ItemsAttributes& ia = p_ecs.GetComponent<ItemsAttributes>(entityID);
			AnimalAttributes& aa = p_ecs.GetComponent<AnimalAttributes>(dragTarget);
			if (aa.current_hunger + ia.gain_amount >= aa.maxHunger)aa.current_hunger = aa.maxHunger;
			else aa.current_hunger += ia.gain_amount;
		}
	}

}
