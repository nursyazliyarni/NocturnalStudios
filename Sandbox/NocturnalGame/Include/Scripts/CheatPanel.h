
/*!------------------------------------------------------------------------------------------------
	File: CheatPanel.h
	Project: NocturnalStudio

	Brief:
		The CheatPanel system manages displaying a cheat panel

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the CheatPanel.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/IScript.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class CheatPanel : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		CheatPanel() {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~CheatPanel() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual CheatPanel* Clone() const override
		{
			return new CheatPanel(*this);
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

		bool Requirements(Entity e) override {
			UNREFERENCED_PARAMETER(e);
			return true;
		}
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "CheatPanel";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		Entity commandMenu{ MAX_ENTITIES };
		bool setupDone{ false }, canCheat{ false };

		void ToggleCheat();
		void DeleteCommandMenu();
	};
}