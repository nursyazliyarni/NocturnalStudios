/*!------------------------------------------------------------------------------------------------
    File: LogicSystem.cpp
    Project: NocturnalStudio

	Brief:
		This file contains the Logic System used for iterating through each entity with a logic
		component, grabbing the vector of scripts from the logic components and
		iterating through each script to call its update functions
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the LogicSystem class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Logic/LogicSystem.h"
#include "GUI/GUI.h"
#include "Timer/Time.h"
#include "LevelEditor/LevelEditor.h"
#include <graphics/Graphics.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Initialize Logic System
	****************************************************************************/
	void LogicSystem::Init()
	{

	}

	/*!*************************************************************************
	Update Loop for Logic System
	****************************************************************************/
	void LogicSystem::Update(float Frametime)
	{
		Timer::GetInstance().Start(Systems::LOGIC);
		Timer::GetInstance().GetDT(Systems::LOGIC);

		//If editor is active, skip logic update
		if (LevelEditor::IsEditorActive())
		{
			Timer::GetInstance().Update(Systems::LOGIC);
			return;
		}

		for (const auto &entity : mEntities)
		{
			// Layering system: skip logic update if this entity is on a disabled layer
			if (p_ecs.HaveComponent<Sprite>(entity))
			{
				int layer = p_ecs.GetComponent<Sprite>(entity).LayerOrder;

				if (layer < 0) layer = 0;
				if (layer >= NocturnalEngine::Graphic::MAX_LAYERS)
					layer = NocturnalEngine::Graphic::MAX_LAYERS - 1;

				if (!NocturnalEngine::Graphic::layerEnabled[layer])
					continue;
			}

			auto& LogicComp = p_ecs.GetComponent<Logic>(entity);
			LogicComp.SetScriptEntity(entity);
			if (!LogicComp.GetScript().empty())
			{
				for (auto i = LogicComp.GetScript().begin(); i != LogicComp.GetScript().end(); ++i)
				{
					if ((p_GUI->check_pause() == false || (*i)->GetScriptName() == "PauseMenu" || (*i)->GetScriptName() == "SliderScript"))
					{
						if (!(*i)->GetScriptInit())
						{
							(*i)->Start();
							(*i)->SetScriptInit();
						}
						if(!(*i)->GetScriptPaused())
							(*i)->Update(Frametime);
					}
				}
			}

			/*if (p_ecs.HaveComponent<AnimalAttributes>(entity)) {
				auto& animal_attributes = p_ecs.GetComponent<AnimalAttributes>(entity);
				if (animal_attributes.is_alive) {
					animal_attributes.current_hunger -= animal_attributes.hungerDecayRate * Frametime * 50;
					animal_attributes.current_health -= animal_attributes.healthDecayRate * Frametime;
					animal_attributes.current_sanity -= animal_attributes.sanityDecayRate * Frametime;
					

					if (animal_attributes.current_hunger <= 0 || animal_attributes.current_health <= 0 || animal_attributes.current_sanity <= 0) {
						animal_attributes.is_alive = false;
					}
				}
			}*/
		}
		Timer::GetInstance().Update(Systems::LOGIC);
	}

	/*!*************************************************************************
	End State for Logic System which will delete all script data
	****************************************************************************/
	void LogicSystem::End()
	{
		for (const auto& entity : mEntities)
		{
			auto& LogicComp = p_ecs.GetComponent<Logic>(entity);
			for (auto i = LogicComp.GetScript().begin(); i != LogicComp.GetScript().end(); ++i)
			{
				(*i)->End();
			}
		}
	}	
}