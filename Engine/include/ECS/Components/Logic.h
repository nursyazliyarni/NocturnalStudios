/*!------------------------------------------------------------------------------------------------
    File: Logic.h
    Project: NocturnalStudio

	Brief:
		Logic Component for all the scripts being used by each entity.
		Contains a vector for scripts which is looped and calls each scripts update
		function
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Logic.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "Scripts/IScript.h"

namespace NocturnalEngine
{

	class IScript;
	class Logic : public IComponent
	{
	public:
		Logic();
		Logic(const Logic& rhs);
		Logic& operator= (const Logic& rhs);
		Logic(Logic&& rhs) noexcept; // move ctor
		Logic& operator=(Logic&& rhs) noexcept; // move assign
		void ToEditor()override;
		~Logic();

		std::vector<IScript*>& GetScript() { return mScriptsVector; }
		void SetScriptEntity(Entity entity);
		void InsertScript(IScript* script, Entity entity);
		void DeleteScript(std::string scriptname);
		void ClearAllScripts();
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
		std::vector<std::string> GetScriptNames() { return mScriptNameVector; }
		void EndAllScripts();

		IScript* GetScriptByName(std::string ScriptName);

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:
		std::vector<std::string> mScriptNameVector;
		std::vector<IScript*> mScriptsVector;

		// Go into IScript.h, use std::string GetScriptName() to find script name
		// Compare script name to mScriptNameVector to find script in vector
		// Use that vector pos to find script in mScriptVector
		// This accesses the object, through the object we can access the member functions.
		// Access state machine through this.
	};
}