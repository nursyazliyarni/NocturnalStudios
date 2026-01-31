
/*!------------------------------------------------------------------------------------------------
	File: DragAndDrop.h
	Project: NocturnalStudio

	Brief:
		 This file contains the DragAndDrop class declaration
		 which is responsible for managing DragAndDrop functionalities within the engine
		 for the DragAndDrop components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the DragAndDrop.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "IScript.h"
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
		Default constructor for Drag & Drop
		****************************************************************************/
		DragAndDrop() : isDragging{ false }, dragTarget{ 0 }, dupeItem{ 0 }, mInput{ InputSystem::GetInstance() }, dragOffset{} {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~DragAndDrop() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual DragAndDrop* Clone() const override
		{
			return new DragAndDrop(*this);
		}
		/*!*************************************************************************
		Start State of Drag & Drop Script
		****************************************************************************/
		virtual void Start() override;
		/*!*************************************************************************
		Update Loop of Drag & Drop Script
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		End State for Drag & Drop
		****************************************************************************/
		virtual void End() override;
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "DragAndDrop";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		bool isDragging;
		Entity dragTarget,dupeItem;
		std::unique_ptr< InputSystem>& mInput;
		vec2D dragOffset;

		void DragCheck();
		Entity DestroyDupeItem();
		bool CheckAnimalCollision();
		void AnimalGain();
		bool BoxCircleCollision(vec2D boxMax,vec2D boxMin,vec2D circleMiddle, float circleRadius);
	};
}