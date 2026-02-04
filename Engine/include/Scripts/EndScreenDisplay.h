
/*!------------------------------------------------------------------------------------------------
	File: EndScreenDisplay.h
	Project: NocturnalStudio

	Brief:
		 This file contains the EndScreenDisplay class declaration
		 which is responsible for managing EndScreenDisplay functionalities within the engine
		 for the EndScreenDisplay components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the EndScreenDisplay.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "IScript.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class EndScreenDisplay : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		EndScreenDisplay() {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~EndScreenDisplay() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual EndScreenDisplay* Clone() const override
		{
			return new EndScreenDisplay(*this);
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
			return "EndScreenDisplay";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		Entity fileBG{}, dayE{}, escapedStats{}, barStats{};
		Transform fileT;
		std::vector<std::pair<Entity,Transform>> paper;

		float timeToFloatIn{1.f }, bgT{ 0.f }, allOtherT{ 0.0f };

		Entity GetEntityByName(const std::string name);
	};
}