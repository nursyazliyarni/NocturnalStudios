/*!------------------------------------------------------------------------------------------------
    File: ButtonResponse.h
    Project: NocturnalStudio

	Brief:
		 This file contains the ButtonResponse class declaration
		 which is responsible for handling button interactions and responses within the engine 
		 for the button components.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the ButtonResponse.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "pch.h"
#include "Scripts/IScript.h"
#include "Input/Input.h"
#include "Math/Math.h"
#include "Window/Window.h"
#include "ECS/Types.h"
#include "ECS/ECS.h"
#include "Logic/States/IStates.h"
#include "ECS/Components/ButtonComponent.h"
namespace NocturnalEngine {
	class ButtonResponse : public IScript
	{
	public:
		ButtonResponse() : clicked{ false }, selected{ false }, initial_hover{ true }, how_to_play{ false } {};
		~ButtonResponse() = default;
		/*!*************************************************************************
		This function initialises the script. As there are no data members that require
		initialization, this function is empty
		****************************************************************************/
		virtual void Start() override;
		/*!*************************************************************************
		This function runs the logic of the script to check for collision and apply
		a counter-force to the entity's current velocity which is calculated based on
		the current velocity and the collision normal
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		This function ends the script by deleting the pointer to this script
		****************************************************************************/
		virtual void End() override;
		/*!*************************************************************************
		This function clones the script by allocating and constructing a copy of this
		script and returning it
		****************************************************************************/
		virtual ButtonResponse* Clone() const override;
		/*!*************************************************************************
		This function returns the purpose of the script as a flag for when it is called
		****************************************************************************/
		virtual std::string GetScriptName() override;

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Collider>(e) && p_ecs.HaveComponent<Tag>(e)&& p_ecs.HaveComponent<Sprite>(e))return true;
			return false;
		}

		/*!*************************************************************************
		This function check if the mouse is within the bounding box of the button 
		****************************************************************************/
		bool is_within_box(glm::vec2 cur, Collider box, Transform trans);

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }
		
		bool clicked = false;
		bool selected = false;
		bool Action = false;

		enum class button_state
		{
			idle,
			select,
			click,
			release,
		};

	private:
		struct button_bb
		{
			vec2D min;
			vec2D max;
		};
		button_bb Button_BoundingBox{};
		button_state curr_state{ button_state::idle};
		std::string originalTexture{};
		bool HTP {false};
		bool initial_hover;
		bool how_to_play;
		bool entercollider{false},leftcollider{ false };

		bool KeyboardSceneSwitch();
		void PlayAudio(bool hover,const ButtonComponent& button);
		void ChangeTexture(const std::string& tex);
		void ScaleButton(float scale, Transform& t);
	};
}


		

		