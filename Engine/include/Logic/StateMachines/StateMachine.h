
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