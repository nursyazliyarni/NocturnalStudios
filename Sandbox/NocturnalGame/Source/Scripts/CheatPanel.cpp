/*!------------------------------------------------------------------------------------------------
	File: CheatPanel.cpp
	Project: NocturnalStudio

	Brief:
		The CheatPanel system manages the displaying of the ingame log which tracks animals statuses
		low hwalth, death etc

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the CheatPanel class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/CheatPanel.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"
#include "Input/Input.h"
#include "GLFW/glfw3.h"
#include "ECS/PrefabManager.h"
#include "ECS/ParentingSystem.h"

namespace NocturnalEngine {

	void CheatPanel::Start()
	{

		DeleteCommandMenu();

		setupDone = true;
	}

	void CheatPanel::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);

		if (!setupDone)Start();
		if (!canCheat)
		{
			if (p_Input->isKeyPressed(GLFW_KEY_BACKSLASH))ToggleCheat();
		}
		else 
		{
			if (p_Input->isKeyPressed(GLFW_KEY_BACKSLASH))ToggleCheat();
		}


	}

	void CheatPanel::ToggleCheat() {
		if (!p_PrefabManager.CheckOrAddPrefabFromName("CheatPanel")) return;

		if (canCheat)
		{
			canCheat = false;
			DeleteCommandMenu();
			return;
		}
		else 
		{
			canCheat = true;
			commandMenu = p_PrefabManager.CloneFromPrefab("CheatPanel");
			return;
		}

	}
	void CheatPanel::DeleteCommandMenu() {
		if (commandMenu != MAX_ENTITIES)
		{
			std::set<Entity> entities{ p_ecs.GetAliveEntitiesSet() };
			for (Entity e : entities)
			{
				if (e == commandMenu) {
					p_Heirarchy.DestroyHierarchy(e);
					break;
				}
			}
			commandMenu = MAX_ENTITIES;
		}
	}
	void CheatPanel::End()
	{

	}




}
