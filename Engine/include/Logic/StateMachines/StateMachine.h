/*!------------------------------------------------------------------------------------------------
    File: StateMachine.h
    Project: NocturnalStudio

	Brief:
		This file contains the logic for the game's state machine.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the StateMachine.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "pch.h"
#include "ECS/Types.h"

namespace NocturnalEngine
{
	class IStates;

	class StateMachine
	{
	public:
		StateMachine(Entity entity);

		~StateMachine() = default;

		void ChangeState(IStates* pNewState);

		void HandleInput(const int& key);

		const Entity& GetEntityID();

		void SetEntityID(Entity& entity) { mEntityID = entity; }

		void OnUpdate(float Frametime);

	private:
		IStates* mCurrState;
		Entity mEntityID;
	};
}