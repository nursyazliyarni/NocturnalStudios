
/*!------------------------------------------------------------------------------------------------
	File: ScriptTemplate.h
	Project: NocturnalStudio

	Brief:
		 

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/IScript.h"



namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class ScriptTemplate : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		ScriptTemplate()  {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~ScriptTemplate() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual ScriptTemplate* Clone() const override
		{
			return new ScriptTemplate(*this);
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
			//Change this to whatever you name your script.
			return "ScriptTemplate";
		}

		bool Requirements(Entity e) override {
			//this will run in the editor. 
			//Add in components that the entity this script is connected to
			//needs to function
			UNREFERENCED_PARAMETER(e);
			return true;
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		
	};
}