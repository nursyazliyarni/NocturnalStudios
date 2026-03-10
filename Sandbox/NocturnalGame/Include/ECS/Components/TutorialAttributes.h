#pragma once
#include "ECS/Components/IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class TutorialAttributes : public IComponent
	{
	public:
		TutorialAttributes();
		~TutorialAttributes() = default;

		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
		bool ToEditor()override;
		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	public:
		int tutorial_order;
	};
}
