
/*!------------------------------------------------------------------------------------------------
	File: SelfDestruct.h
	Project: NocturnalStudio

	Brief:
		 This file contains the SelfDestruct class declaration
		 which is responsible for managing SelfDestruct functionalities within the engine
		 for the SelfDestruct components.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the SelfDestruct.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/IScript.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class SelfDestruct : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		SelfDestruct() {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~SelfDestruct() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual SelfDestruct* Clone() const override
		{
			return new SelfDestruct(*this);
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
			if (p_ecs.HaveComponent<Sprite>(e))return true;
			return false;
			
		}
		std::vector<ScriptField> GetFields() override {
			return {
				{ "TimeTillDie",    &timeTillDie    }
			};
		}
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "SelfDestruct";
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		float timeTillDie{0.5f}, timeSince{};
	};
}
