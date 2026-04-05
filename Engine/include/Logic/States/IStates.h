/*!------------------------------------------------------------------------------------------------
    File: IStates.h
    Project: NocturnalStudio

    Date:
        2026-04-05

    Brief:
        Declares the base interface for logic states.

    Primary Author:
         Nur Syazliyarni (100%)
        - Added the file header.
        - Email: nursyazliyarni.b

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
