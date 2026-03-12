
/*!------------------------------------------------------------------------------------------------
	File: AnimalLogic.h
	Project: NocturnalStudio

	Brief:
		 This file contains the AnimalLogic class declaration
		 which is responsible for managing AnimalLogic functionalities within the engine
		 for the AnimalLogic components.

	Primary Author:
		 ANG YU XUAN (70%)
		- Created the AnimalLogic class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Adjusted health decay behavior per animal.
		Sarah (25%)
		- Make improvements and fix error with this AnimalLogic script.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/IScript.h"
#include "ECS/Components/AnimalAttributes.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Class for Drag & Drop Script
	****************************************************************************/
	class AnimalLogic : public IScript
	{
	public:

		struct AttributesDecreaseChance
		{
			float  hunger{ 10.f }, hp{ 10.f }, sanity{ 20.f };
		};
		struct TimeSinceCheck {
			float  hunger{ 0.f }, hp{ 0.f }, sanity{ 0.f };
		};
		/*!*************************************************************************
		Default constructor for Drag & Drop
		****************************************************************************/
		AnimalLogic()  {}
		/*!*************************************************************************
		Default destructor for Drag & Drop
		****************************************************************************/
		~AnimalLogic() = default;
		/*!*************************************************************************
		Returns a new copy of Drag & Drop Script
		****************************************************************************/
		virtual AnimalLogic* Clone() const override
		{
			return new AnimalLogic(*this);
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
			return "AnimalLogic";
		}

		bool Requirements(Entity e) override {
			if (p_ecs.HaveComponent<AnimalAttributes>(e))return true;
			return false;
		}

		virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

		virtual Entity& GetScriptEntityID() override { return entityID; }

	private:
		AttributesDecreaseChance chance;
		TimeSinceCheck time;

		Entity toyE{ MAX_ENTITIES };

		float checkPeriod{};
		std::string ogSpritename{};
		void CheckAndDecreaseStats(float& time,float& chance,double decayRate,double& attributeValue, float frametime,double decreaseScale=1.0);

		void LogLowHealth(AnimalAttributes& aA);
		void ChangeAnim(AnimalAttributes& aA);
		// sets on start ,//per second			//max
		float orgXPos{}, xMovementSpeed{0.05f}, maxXMovement{ 0.05f };
		void MoveAnimalLeftRight(float Frametime);

		bool spriteDeactivated{ false };

		
	};
}