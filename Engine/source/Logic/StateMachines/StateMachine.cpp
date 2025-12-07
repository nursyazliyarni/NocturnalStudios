
#include "pch.h"
#include "Logic/StateMachines/StateMachine.h"
#include "Logic/States/IStates.h"

namespace NocturnalEngine
{
	StateMachine::StateMachine(Entity entity) : mCurrState{}, mEntityID{ entity } {}
	
	void StateMachine::ChangeState(IStates* pNewState)
	{
		if (mCurrState != nullptr)
		{
			mCurrState->OnExit(this);
		}
		mCurrState = pNewState;
		mCurrState->OnEnter(this);
	}

	void StateMachine::HandleInput(const int& key)
	{
		IStates* state = mCurrState->HandleInput(this, key);
		if (state != nullptr)
		{
			ChangeState(state);
		}
		 
	}

	const Entity& StateMachine::GetEntityID()
	{
		return mEntityID;
	}

	void StateMachine::OnUpdate(float Frametime)
	{
		mCurrState->OnUpdate(this, Frametime);
	}
}