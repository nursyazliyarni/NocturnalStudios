/*!------------------------------------------------------------------------------------------------
    File: IScript.h
    Project: NocturnalStudio

	Brief:
		This file contains the abstract IScript class declaration
		which is responsible for defining the interface for all scripts
		within the engine.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the IScript.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Types.h"
#include "ECS/ECS.h"
#include "Serialization/JSONserialization.h"
#include <variant>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Interface for Script
	When Adding a new script
	1) Ensure the Logic Component has the inserted script when deserializing
	2) Ensure the Level Editor has the new script that can be inserted
	****************************************************************************/
	class IScript :public JSONSerializer
	{
	public:

		struct ScriptField {
			std::string name;
			std::variant<float*, int*, bool*,std::string*> ptr;
		};
		virtual std::vector<ScriptField> GetFields() { return {}; }
		virtual ~IScript() = default;
		virtual void Start() {};
		virtual void Update(float Frametime) { UNREFERENCED_PARAMETER(Frametime); };
		virtual void End() {};
		virtual IScript* Clone() const = 0;
		virtual std::string GetScriptName() = 0;

		virtual bool ToEditor();
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override;
		virtual bool Deserialize(const rapidjson::Value& obj) override;


		virtual void SetScriptEntityID(Entity& entity) = 0;
		virtual Entity & GetScriptEntityID() = 0;

		void SetScriptInit()
		{
			if (!mScriptInit)
			{
				mScriptInit = true;
			}
		}

		bool GetScriptInit()
		{
			return mScriptInit;
		}

		void TogglePauseScript()
		{
			mScriptPause == true ? false : true;
		}

		bool GetScriptPaused()
		{
			return mScriptPause;
		}

		void SetScriptPause(bool value)
		{
			mScriptPause = value;
		}

		bool mScriptPause{ false };

		virtual bool Requirements(Entity) = 0;

	protected:
		Entity entityID{};

		bool mScriptInit{ false };

	};
}