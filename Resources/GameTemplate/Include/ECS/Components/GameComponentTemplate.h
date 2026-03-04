/*!------------------------------------------------------------------------------------------------
    File: GameComponentTemplate.h
    Project: TEMPLATE_PROJECT_NAME

	Brief:
		
		
    Primary Author:
         

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#pragma once
#include "ECS/Components/IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class GameComponentTemplate : public IComponent
	{
	public:
		
		GameComponentTemplate();

		~GameComponentTemplate() = default;

		virtual bool Deserialize(const rapidjson::Value& obj);

		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		virtual bool ToEditor() override;

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
