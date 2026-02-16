
/*!------------------------------------------------------------------------------------------------
	File: TimerDisplay.h
	Project: NocturnalStudio

	Brief:
		 This file contains the TimerDisplay class declaration
		 which is responsible for managing TimerDisplay functionalities within the engine
		 for the TimerDisplay components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the TimerDisplay.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "IScript.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class TimerDisplay : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		TimerDisplay() {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~TimerDisplay() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual TimerDisplay* Clone() const override
		{
			return new TimerDisplay(*this);
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
			return "TimerDisplay";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		float timeScale{};
	};
}