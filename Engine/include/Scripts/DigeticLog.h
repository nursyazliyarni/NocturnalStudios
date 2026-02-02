
/*!------------------------------------------------------------------------------------------------
	File: DigeticLog.h
	Project: NocturnalStudio

	Brief:
		 This file contains the DigeticLog class declaration
		 which is responsible for managing DigeticLog functionalities within the engine
		 for the DigeticLog components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the DigeticLog.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "IScript.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class DigeticLog : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		DigeticLog() {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~DigeticLog() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual DigeticLog* Clone() const override
		{
			return new DigeticLog(*this);
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
			return "DigeticLog";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:	
		int logSpaces{ 0 };
		float textWorldHeight{ };
		std::vector<Entity> logTxtObjs{};
		bool setupDone{ false };
	};
}