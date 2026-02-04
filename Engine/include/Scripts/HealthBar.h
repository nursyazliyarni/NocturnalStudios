
/*!------------------------------------------------------------------------------------------------
	File: HealthBar.h
	Project: NocturnalStudio

	Brief:
		 This file contains the HealthBar class declaration
		 which is responsible for managing HealthBar functionalities within the engine
		 for the HealthBar components.

	Primary Author:
		 ANG YU XUAN (100%)
		- Created the HealthBar.h file and its function specification.

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
	class HealthBar : public IScript
	{
	public:
		

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		HealthBar() :linkedAnimal{ 0 }, origin{} {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~HealthBar() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual HealthBar* Clone() const override
		{
			return new HealthBar(*this);
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
			return "HealthBar";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		Entity linkedAnimal;
		Transform origin;
		

		int findAnimalTries{};
		void FindAnimal();
	};
}