#pragma once

#include "pch.h"
#include "Scripts/IScript.h"
#include "Input/Input.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Tutorial Logic Script
	****************************************************************************/
	class TutorialLogic : public IScript
	{
	public:

		/*!*************************************************************************
		Default constructor for Tutorial Logic
		****************************************************************************/
		TutorialLogic() {}
		/*!*************************************************************************
		Default destructor for Tutorial Logic
		****************************************************************************/
		~TutorialLogic() = default;
		/*!*************************************************************************
		Returns a new copy of Tutorial Logic Script
		****************************************************************************/
		virtual TutorialLogic* Clone() const override
		{
			return new TutorialLogic(*this);
		}
		/*!*************************************************************************
		Start State of Tutorial Logic Script
		****************************************************************************/
		virtual void Start() override;
		/*!*************************************************************************
		Update Loop of Tutorial Logic Script
		****************************************************************************/
		virtual void Update(float Frametime) override;
		/*!*************************************************************************
		End State for Tutorial Logic
		****************************************************************************/
		virtual void End() override;
		/*!*************************************************************************
		Returns the name of Script
		****************************************************************************/
		virtual std::string GetScriptName() override
		{
			return "TutorialLogic";
		}

		bool Requirements(Entity e) override {
			UNREFERENCED_PARAMETER(e);
			//if (p_ecs.HaveComponent<Collider>(e)) { return true; }
			return true;
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
	};
}