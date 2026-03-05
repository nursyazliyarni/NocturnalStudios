/*!------------------------------------------------------------------------------------------------
	File: ButtonComponent.h
	Project: NocturnalStudio

	Brief:
		Contains the attributes related to animals in the ECS

	Primary Author:
		 Ang Yu Xuan (100%)
		- Created the ButtonComponent.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS/Components/IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class ButtonComponent : public IComponent
	{
	public:
		
		ButtonComponent() {}
		~ButtonComponent() = default;

		virtual bool Deserialize(const rapidjson::Value& obj) {}

		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const {}

		virtual bool ToEditor() override {}

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

		

	private:
		enum class ButtonActions {
			None = 0,
			ReadOnly = 1 << 0, // Binary 0001
			Write = 1 << 1, // Binary 0010
			Append = 1 << 2, // Binary 0100
			Binary = 1 << 3  // Binary 1000
		};
	};
}
