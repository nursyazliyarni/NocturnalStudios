#pragma once
/*!------------------------------------------------------------------------------------------------
	File: DragAndDrop.h
	Project: NocturnalStudio

	Brief:
		 This file contains the CutScene class declaration
		 which is responsible for managing cutscene functionalities within the engine
		 for the cutscene components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the DragAndDrop.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IScript.h"
#include "Input/Input.h"
#include "Graphics/Camera2D.h"
#include "Graphics/Graphics.h"
#include "ECS/SceneManager.h"
#include "Input/Input.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class DragAndDrop : public IScript
	{
	public:
		/*!*************************************************************************
		Default constructor for Cutscenes
		****************************************************************************/
		DragAndDrop() : timer(0), draggedItem{ 0 }, mSwitchAudio{ 1 } {}
		/*!*************************************************************************
		Default destructor for Cutscenes
		****************************************************************************/
		~DragAndDrop() = default;
		/*!*************************************************************************
		Returns a new copy of Cutscenes Script
		****************************************************************************/
		virtual DragAndDrop* Clone() const override
		{
			return new DragAndDrop(*this);
		}
		/*!*************************************************************************
		Start State of Cutscenes Script
		****************************************************************************/
		virtual void Start() override
		{

		}
		/*!*************************************************************************
		Update Loop of Cutscenes Script
		****************************************************************************/
		virtual void Update(float Frametime) override
		{
	


		}
		/*!*************************************************************************
		End State for Cutscenes
		****************************************************************************/
		virtual void End() override
		{

		}
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "CutScene";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		float timer;
		Entity draggedItem;
		int mSwitchAudio;
	};
}