
#pragma once
#include "pch.h"
#include "Input/Input.h"
#include "GLFW/glfw3.h"
#include "ECS/ECS.h"
#include "Logic/StateMachines/StateMachine.h"

namespace NocturnalEngine
{
	class StateMachine;

	class IStates
	{
	public:
		virtual ~IStates() = default;
		virtual IStates* HandleInput(StateMachine* stateMachine, const int& key) = 0;
		virtual void OnEnter(StateMachine* stateMachine) = 0;
		virtual void OnUpdate(StateMachine* stateMachine, float Frametime) = 0;
		virtual void OnExit(StateMachine* stateMachine) = 0;
	};

}