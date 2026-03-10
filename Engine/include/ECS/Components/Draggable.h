/*!------------------------------------------------------------------------------------------------
	File: Draggable.h
	Project: NocturnalStudio

	Brief:
		This file contains the component declaration as well as member function
		definitions for the collider component of the game engine.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the Draggable.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"

namespace NocturnalEngine
{
	class Draggable : public IComponent
	{
	public:

		Draggable();
		~Draggable() = default;
		bool ToEditor()override;
		/*!*************************************************************************
		This function de-serializes the level colliders from the given level json file
		****************************************************************************/
		virtual bool Deserialize(const rapidjson::Value& obj);
		/*!*************************************************************************
		This function saves the level information by serializing it to the level json
		file
		****************************************************************************/
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		bool &IsTarget() { return isTarget; }


		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:
		bool isTarget{};
	};
}
