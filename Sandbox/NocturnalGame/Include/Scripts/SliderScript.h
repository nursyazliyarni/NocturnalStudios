/*!------------------------------------------------------------------------------------------------
    File: SliderScript.h
    Project: NocturnalStudio

	Brief:
		 This file contains the SliderScript class declaration
		 which is responsible for handling slider interactions and responses within the engine 
		 for the slider components.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the SliderScript.h file and its function specification.

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
#include <vector>
namespace NocturnalEngine {
	class SliderScript : public IScript
	{
	public:
		SliderScript() = default;
		~SliderScript() = default;
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
		This function ends the script by deleting the pointer to this scriptv
		****************************************************************************/
		virtual void End() override;

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<Collider>(e) && p_ecs.HaveComponent<Tag>(e)
				&& p_ecs.HaveComponent<Sprite>(e) && p_ecs.HaveComponent<NameTag>(e))return true;
			return false;
		}

		/*!*************************************************************************
		This function clones the script by allocating and constructing a copy of this
		script and returning it
		****************************************************************************/
		virtual SliderScript* Clone() const override;
		/*!*************************************************************************
		This function returns the purpose of the script as a flag for when it is called
		****************************************************************************/
		virtual std::string GetScriptName() override;

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

		/*!*************************************************************************
		This function check if the mouse is within the bounding box of the button
		****************************************************************************/
		bool is_within_box(glm::vec2 cur, Collider box, Transform trans);

		bool clicked{};
		bool selected{};
		bool SliderCali{};
		bool limitset{};
		bool ingame{};

	private:
		struct slider_bb
		{
			vec2D min;
			vec2D max;
		};
		vec2D def_position{};
		slider_bb limit{};

		float masterPercentage{ 100.f }, backgroundPercentage{ 100.f }, effectPrecentage{ 100.f };

	};
}

