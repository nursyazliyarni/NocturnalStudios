/*!------------------------------------------------------------------------------------------------
    File: CameraComponent.h
    Project: NocturnalStudio

	Brief:
		Contains the component related to camera in the ECS
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the CameraComponent.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "glm/glm.hpp"

namespace NocturnalEngine {

	class CameraComponent : public IComponent
	{
	public:
		CameraComponent();
		~CameraComponent() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }
	private:
	};
}
