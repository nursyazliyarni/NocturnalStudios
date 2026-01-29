/*!------------------------------------------------------------------------------------------------
    File: SceneManager.h
    Project: NocturnalStudio

	Brief:
		This file contains all the functions for loading and unloading the
		level data into a JSON file. Currently on application end the ecs will be
		saved into a json file for future loading
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the SceneManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS.h"
#include "ComponentManager.h"
#include "EntityManager.h"
#include "System/SystemManager.h"
#include "Types.h"
#include "pch.h"
#include "Serialization/JSONserialization.h"
#include "Serialization/JSONincludes.h"

#define p_Scene NocturnalEngine::SceneManager::GetInstance()

namespace NocturnalEngine
{

	class SceneManager : public JSONSerializer
	{
	public:
		/*!*************************************************************************
		Returns an instance of the SceneManager
		****************************************************************************/
		static std::unique_ptr<SceneManager>& GetInstance();

		/*!*************************************************************************
		Initialize all registered components
		****************************************************************************/
		void Init();
		/*!*************************************************************************
		Deserializes ECS entities and data from JSON file to build ECS (File loading)
		****************************************************************************/
		virtual bool Deserialize(const rapidjson::Value& obj);
		/*!*************************************************************************
		Serializes ECS entities and data to JSON file to save ECS (File saving)
		****************************************************************************/
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		// temp: scene to load code
		void setSceneToLoad(std::string s);
		void checkForSceneToLoad();
		void changeScene();

		bool isGameplay{false};
	private:
		// Unique pointer to SceneManager
		static std::unique_ptr<SceneManager> m_instance;

		std::string sceneToLoad;
	};
}