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
		UNREFERENCED_PARAMETER(Frametime);
		
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
					p_ecs.AddComponent<Collider>(dupeItem, p_ecs.GetComponent<Collider>(entityID));

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
			Collider& dupeC = p_ecs.GetComponent<Collider>(dupeItem);
			Collider& tC= p_ecs.GetComponent<Collider>(dragTarget);
			Transform& tT = p_ecs.GetComponent<Transform>(dragTarget);

			for(int dupei=0;dupei<2;dupei++){
				vec2D dupeOffsetPos{ dupeT.GetPos().x + dupeC[dupei].mOffset.x ,dupeT.GetPos().y + dupeC[dupei].mOffset.y };
				bool isDupeBox = (dupeC[dupei].mCol == Collider::ColliderType::rect || dupeC[dupei].mCol == Collider::ColliderType::box);
				vec2D dupeMinPt{ }, dupeMaxPt{  };

				
				if (isDupeBox) {
					dupeMinPt = {dupeOffsetPos.x - dupeC[dupei].mMin.x, dupeOffsetPos.y - dupeC[dupei].mMin.y};
					dupeMaxPt = { dupeOffsetPos.x + dupeC[dupei].mMax.x, dupeOffsetPos.y + dupeC[dupei].mMax.y };
				}

			for (int targeti = 0; targeti < 2; targeti++)
			{
				//fuck this im too tired
				vec2D targetOffsetPos{ tT.GetPos().x + tC[targeti].mOffset.x ,tT.GetPos().y + tC[targeti].mOffset.y };
				switch (tC[targeti].mCol)
				{
				case Collider::ColliderType::circle:
				case Collider::ColliderType::bubble:
					if (isDupeBox)
					{
						return BoxCircleCollision(dupeMaxPt, dupeMinPt, targetOffsetPos, tC[targeti].mRadius);
					}
					else if(dupeC[dupei].mCol == Collider::ColliderType::circle || dupeC[dupei].mCol == Collider::ColliderType::bubble) {
						if (distance(dupeT.GetMinPos(), targetOffsetPos) < tC[targeti].mRadius || distance(dupeT.GetMaxPos(), targetOffsetPos) < tC[targeti].mRadius) {
							return true;
						}
					}
					
					break;
				case Collider::ColliderType::rect:
				case Collider::ColliderType::box:
					vec2D targetMinPt{ targetOffsetPos.x - tC[targeti].mMin.x,targetOffsetPos.y - tC[targeti].mMin.y }, targetMaxPt{ targetOffsetPos.x + tC[targeti].mMax.x,targetOffsetPos.y + tC[targeti].mMax.y };
					if (isDupeBox)
					{
						
						if (dupeMinPt.x<targetMaxPt.x && dupeMaxPt.x>targetMinPt.x &&
							dupeMinPt.y<targetMaxPt.y && dupeMaxPt.y>targetMinPt.y)
						{
							//std::cout<<"dupe scale"
							//std::cout << "Animal minX:" << minPt.x << " minY:" << minPt.y << " maxX:" << maxPt.x << " maxY:" << maxPt.y << std::endl;
							//std::cout << "Dupe minX:" << dupeT.GetMinPos().x << " minY:" << dupeT.GetMinPos().y << " maxX:" << dupeT.GetMaxPos().x << " maxY:" << dupeT.GetMaxPos().y << std::endl;
							return true;

						}
					}
					else if (dupeC[dupei].mCol == Collider::ColliderType::circle || dupeC[dupei].mCol == Collider::ColliderType::bubble) {
						return BoxCircleCollision(targetMaxPt, targetMinPt, dupeOffsetPos, dupeC[targeti].mRadius);
					}
					
					break;
				}
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
			if (!aa.is_alive)return;
			if (aa.current_hunger + ia.gain_amount >= aa.maxHunger)aa.current_hunger = aa.maxHunger;
			else aa.current_hunger += ia.gain_amount;
		}
	}
	bool DragAndDrop::BoxCircleCollision(vec2D boxMax, vec2D boxMin, vec2D circleMiddle, float circleRadius) {
		vec2D closest{ std::clamp(circleMiddle.x, boxMin.x, boxMax.x), std::clamp(circleMiddle.y, boxMin.y, boxMax.y) };

		vec2D delta{ circleMiddle - closest };
		return dotProduct(delta, delta) <= circleRadius * circleRadius;
	}
}
