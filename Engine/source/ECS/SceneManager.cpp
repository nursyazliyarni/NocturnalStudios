/*!------------------------------------------------------------------------------------------------
    File: SceneManager.cpp
    Project: NocturnalStudio

	Brief:
		This file contains all the functions for loading and unloading the
		level data into a JSON file. Currently on application end the ecs will be
		saved into a json file for future loading
	
    Primary Author:
         LAU XIANG YANG (75%)
		- Created the scene manager and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added Text component handling in scene load/save.
		LAYCOCK ENHUI SARAH (20%)
		- Redid the serialization and deserialization
		- In serialization & deserialization, added implementation for prefabs and parenting

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/SceneManager.h"
#include "Graphics/Graphics.h"
#include "Audio/AudioEngine.h"
#include "ECS/EngineComponentReg.h"
#include "Core/MemoryManager.h"
#include "System/ServiceLocator.h"
#include "ECS/ParentingSystem.h"
#include <unordered_map>
#include <array>


namespace NocturnalEngine
{
	//Unique pointer to instance of Scene
	std::unique_ptr<SceneManager> SceneManager::m_instance;

	static std::unordered_map<std::string,
		std::array<bool, NocturnalEngine::Graphic::MAX_LAYERS>> gSceneLayerState_Runtime;

	// Legacy global scene-path tracking kept for reference after moving it into SceneManager state.
	// static std::string gCurrentScenePath = "";


	/*!*************************************************************************
	Returns an instance of the SceneManager
	****************************************************************************/
	std::unique_ptr<SceneManager>& SceneManager::GetInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = std::make_unique<SceneManager>();
		}
		return m_instance;
	}

	/*!*************************************************************************
	Initialize the scenemanager
	****************************************************************************/
	void SceneManager::Init()
	{
		sceneToLoad = "";
		mQueuedSceneToLoad.clear();
		mCurrentScenePath.clear();
		mTransitionPhase = TransitionPhase::None;
		mTransitionAlpha = 0.0f;
	}


	/*!*************************************************************************
	Deserialize- has backwards compatability with the old method of serialization
	****************************************************************************/
	bool SceneManager::Deserialize(const rapidjson::Value& obj)
	{
		changeScene();

		if (obj.HasMember("EntitySignatures"))
		{
			return OldDeserialize(obj);
		}
		else {
			return NewDeserialize(obj);
		}
	}


	/*!*************************************************************************
	Serializes ECS entities and data to JSON file to save ECS (File saving)
	****************************************************************************/
	bool SceneManager::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		return NewSerialize(writer);


	}
}

// Legacy direct scene-load path kept for reference after adding the cutscene-to-tutorial fade.
//
// void NocturnalEngine::SceneManager::setSceneToLoad(std::string s)
// {
// 	sceneToLoad = s;
// }
//
// void NocturnalEngine::SceneManager::checkForSceneToLoad()
// {
// 	if (sceneToLoad != "")
// 	{
// 		changeScene();
//
// 		/*if (!isGameplay) {
// 			isGameplay = true;
// 			DeserializeFromFile(sceneToLoad);
// 		}*/
//
// 		if (sceneToLoad == "Assets/Scene/Menu.json")
// 		{
// 			isGameplay = true;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/FlamingoScene.json")
// 		{
// 			isGameplay = true;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/HorseScene.json")
// 		{
// 			isGameplay = true;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/SealScene.json")
// 		{
// 			isGameplay = true;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/OwlScene.json")
// 		{
// 			isGameplay = true;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/Menu.json")
// 		{
// 			isGameplay = false;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/Options.json")
// 		{
// 			isGameplay = false;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 		if (sceneToLoad == "Assets/Scene/Game_Over.json")
// 		{
// 			isGameplay = false;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
// 		if (sceneToLoad == "Assets/Scene/LoseScene.json")
// 		{
// 			isGameplay = false;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
// 		if (sceneToLoad == "Assets/Scene/WinScene.json")
// 		{
// 			isGameplay = false;
// 			Graphic::camera.SetZoomLevel(1.0f);
// 			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 		}
//
// 			if (sceneToLoad == "Assets/Scene/CutScene.json")
// 			{
// 				isGameplay = false;
// 				Graphic::camera.SetZoomLevel(1.0f);
// 				Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 			}
//
// 		if (sceneToLoad == "Assets/Scene/HowToPlay.json")
// 		{
// 			isGameplay = false;
// 			if (sceneToLoad == "Assets/Scene/CombinedScene.json")
// 			{
// 				isGameplay = true;
// 				Graphic::camera.SetZoomLevel(1.0f);
// 				Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
// 			}
// 		}
//
// 		//SAVE current scene layer state before switching away
// 		if (!gCurrentScenePath.empty())
// 		{
// 			auto& saved = gSceneLayerState_Runtime[gCurrentScenePath];
// 			for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
// 				saved[l] = NocturnalEngine::Graphic::layerEnabled[l];
// 		}
//
// 		DeserializeFromFile(sceneToLoad);
// 		sceneToLoad = "";
//
// 		gCurrentScenePath = sceneToLoad;
//
// 		// RESTORE layer state for newly loaded scene 
// 		auto it = gSceneLayerState_Runtime.find(gCurrentScenePath);
// 		if (it != gSceneLayerState_Runtime.end())
// 		{
// 			for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
// 				NocturnalEngine::Graphic::layerEnabled[l] = it->second[l];
// 		}
// 		else
// 		{
// 			for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
// 				NocturnalEngine::Graphic::layerEnabled[l] = true;
// 		}
//
// 		sceneToLoad = "";
// 	}
// }
//
void NocturnalEngine::SceneManager::setSceneToLoad(std::string s)
{
	if (ShouldUseFadeTransition(mCurrentScenePath, s))
	{
		// Queue the tutorial load so the current cutscene scene can fade fully to black first.
		mQueuedSceneToLoad = s;
		mTransitionPhase = TransitionPhase::FadeOut;
		if (sceneToLoad.empty())
		{
			sceneToLoad = s;
		}
		return;
	}

	sceneToLoad = s;
}

void NocturnalEngine::SceneManager::checkForSceneToLoad()
{
	constexpr float kSceneTransitionDuration = 0.2f;
	const float dt = Timer::GetInstance().GetGlobalDT();

	if (mTransitionPhase == TransitionPhase::FadeOut)
	{
		// Fade out the current scene before switching to the tutorial scene.
		mTransitionAlpha = std::min(1.0f, mTransitionAlpha + (dt / kSceneTransitionDuration));
		if (mTransitionAlpha >= 1.0f && !mQueuedSceneToLoad.empty())
		{
			LoadSceneNow(mQueuedSceneToLoad);
			mQueuedSceneToLoad.clear();
			sceneToLoad.clear();
			mTransitionPhase = TransitionPhase::FadeIn;
		}
		return;
	}

	if (mTransitionPhase == TransitionPhase::FadeIn)
	{
		// Fade back in once the tutorial scene has finished loading.
		mTransitionAlpha = std::max(0.0f, mTransitionAlpha - (dt / kSceneTransitionDuration));
		if (mTransitionAlpha <= 0.0f)
		{
			mTransitionAlpha = 0.0f;
			mTransitionPhase = TransitionPhase::None;
		}
		return;
	}

	if (sceneToLoad != "")
	{
		LoadSceneNow(sceneToLoad);
		sceneToLoad.clear();
	}
}

float NocturnalEngine::SceneManager::GetSceneTransitionAlpha() const
{
	return mTransitionAlpha;
}

bool NocturnalEngine::SceneManager::ShouldUseFadeTransition(const std::string& currentScene, const std::string& nextScene) const
{
	// Only the cutscene-to-tutorial handoff uses this temporary black-screen transition.
	return currentScene == "Assets/Scene/CutScene.json" && nextScene == "Assets/Scene/Tutorial.json";
}

void NocturnalEngine::SceneManager::LoadSceneNow(const std::string& path)
{
	changeScene();

	if (path == "Assets/Scene/Menu.json")
	{
		isGameplay = true;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/FlamingoScene.json")
	{
		isGameplay = true;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/HorseScene.json")
	{
		isGameplay = true;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/SealScene.json")
	{
		isGameplay = true;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/OwlScene.json")
	{
		isGameplay = true;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/Menu.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/Options.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/Game_Over.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}
	if (path == "Assets/Scene/LoseScene.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}
	if (path == "Assets/Scene/WinScene.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/CutScene.json")
	{
		isGameplay = false;
		Graphic::camera.SetZoomLevel(1.0f);
		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
	}

	if (path == "Assets/Scene/HowToPlay.json")
	{
		isGameplay = false;
		if (path == "Assets/Scene/CombinedScene.json")
		{
			isGameplay = true;
			Graphic::camera.SetZoomLevel(1.0f);
			Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });
		}
	}

	if (!mCurrentScenePath.empty())
	{
		auto& saved = gSceneLayerState_Runtime[mCurrentScenePath];
		for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
			saved[l] = NocturnalEngine::Graphic::layerEnabled[l];
	}

	DeserializeFromFile(path);
	// Track the live scene path after the load so future transitions know the current source scene.
	mCurrentScenePath = path;

	auto it = gSceneLayerState_Runtime.find(mCurrentScenePath);
	if (it != gSceneLayerState_Runtime.end())
	{
		for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
			NocturnalEngine::Graphic::layerEnabled[l] = it->second[l];
	}
	else
	{
		for (int l = 0; l < NocturnalEngine::Graphic::MAX_LAYERS; ++l)
			NocturnalEngine::Graphic::layerEnabled[l] = true;
	}
}

void NocturnalEngine::SceneManager::changeScene()
{
	for (Entity i = 0; i < p_ecs.GetTotalSceneEntities(); ++i)
	{
		if (p_ecs.HaveComponent<Logic>(i))
		{
			p_ecs.GetComponent<Logic>(i).EndAllScripts();
		}
	}
	p_PrefabManager.ClearRegistry();
	p_Heirarchy.ClearSceneEntities();
	p_ecs.ClearAllComponents();
	p_ecs.ResetAllEntitySignatures();
	p_ecs.ResetEntities();

}

/*!*************************************************************************
This new deserialize makes use of the component registry to process the scene
files
****************************************************************************/
bool NocturnalEngine::SceneManager::NewDeserialize(const rapidjson::Value& obj) {
	for (int i = 0; i < NocturnalEngine::Graphic::MAX_LAYERS; ++i)
		NocturnalEngine::Graphic::layerDrawOrder[i] = i;

	//load if exists
	if (obj.HasMember("LayerDrawOrder") && obj["LayerDrawOrder"].IsArray())
	{
		const auto& arr = obj["LayerDrawOrder"].GetArray();
		int n = (int)arr.Size();
		if (n > NocturnalEngine::Graphic::MAX_LAYERS) n = NocturnalEngine::Graphic::MAX_LAYERS;

		for (int i = 0; i < n; ++i)
			NocturnalEngine::Graphic::layerDrawOrder[i] = arr[i].GetInt();
	}
	


	if (obj.HasMember("Entities")&&obj["Entities"].IsArray())
	{
		if (p_MemoryManager.IsEnabled())
		{
			p_ecs.PrepareMemoryForEntities(obj["Entities"].Size());
		}

		std::unordered_map<Entity, Entity> entityOldIDMap{};



		PrefabManager& prefabs{ p_PrefabManager };
		for (const rapidjson::Value& ent:obj["Entities"].GetArray())
		{
			DeserializeEntity(ent, prefabs, entityOldIDMap);
		}

	}
	return true;
}

NocturnalEngine::Entity NocturnalEngine::SceneManager::DeserializeEntity(const rapidjson::Value& ent, PrefabManager& prefabs, std::unordered_map<Entity, Entity>& entityOldIDMap, Entity parent) const {

	const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>& ecr = p_ECompReg.GetAll();

	ComponentRegistry* gameComp = ServiceLocator::Get().GetGameComponents();
	const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>* gcr =
		gameComp
		? &gameComp->GetAll()
		: nullptr;
	bool fromPrefab{ ent.HasMember("Prefab") };

	Entity e{ MAX_ENTITIES };

	if (fromPrefab) {
		std::string prefabName{};
		if (!ent["Prefab"].IsString()) {
			fromPrefab = false;
		}
		else {
			prefabName = ent["Prefab"].GetString();
			fromPrefab = prefabs.CheckOrAddPrefabFromName(prefabName);
		}

		if (fromPrefab) {
			e = prefabs.CloneFromPrefab(prefabName);
		}
	}

	if (!ent.HasMember("Components") || !ent["Components"].IsArray())return e;

	if (!fromPrefab)
	{
		e = p_ecs.CreateEntity();
	}

	if (e == MAX_ENTITIES) return e;

	for (const rapidjson::Value& comps : ent["Components"].GetArray())
	{
		if (!comps.HasMember("Component") || !comps["Component"].IsString())continue;

		std::string componentName = comps["Component"].GetString();
		if (ecr.contains(componentName)) {
			ecr.at(componentName).deserializeAndAddFunc(e, comps);
		}
		else if (gcr && gcr->contains(componentName))
		{
			gcr->at(componentName).deserializeAndAddFunc(e, comps);
		}


	}

	if (ent.HasMember("ID") && ent["ID"].IsUint()) {
		entityOldIDMap.emplace(static_cast<Entity>(ent["ID"].GetUint()), e);
	}
	if (parent!=MAX_ENTITIES)
	{
		p_Heirarchy.SetParent(e, parent,false,true);
	}
	
	if (ent.HasMember("Children")&& ent["Children"].IsArray())
	{
		for (const rapidjson::Value& chd : ent["Children"].GetArray())
		{
			DeserializeEntity(chd, prefabs, entityOldIDMap, e);
		}
	}





	return e;
}

bool NocturnalEngine::SceneManager::OldDeserialize(const rapidjson::Value& obj) {
	//default
	for (int i = 0; i < NocturnalEngine::Graphic::MAX_LAYERS; ++i)
		NocturnalEngine::Graphic::layerDrawOrder[i] = i;

	//load if exists
	if (obj.HasMember("LayerDrawOrder") && obj["LayerDrawOrder"].IsArray())
	{
		const auto& arr = obj["LayerDrawOrder"].GetArray();
		int n = (int)arr.Size();
		if (n > NocturnalEngine::Graphic::MAX_LAYERS) n = NocturnalEngine::Graphic::MAX_LAYERS;

		for (int i = 0; i < n; ++i)
			NocturnalEngine::Graphic::layerDrawOrder[i] = arr[i].GetInt();
	}

	if (p_MemoryManager.IsEnabled())
	{
		p_ecs.PrepareMemoryForEntities(obj["Number of Entities"]["Entities"].GetUint());
	}
	p_ecs.SetTotalEntitiesForWorldBuild(obj["Number of Entities"]["Entities"].GetUint());

	ComponentRegistry* gameComp = ServiceLocator::Get().GetGameComponents();
	const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>* gcr =
		gameComp
		? &gameComp->GetAll()
		: nullptr;
	auto aliveTotal = obj["Number of Entities"]["Entities"].GetUint();
	Entity j = 0;
	while (j < aliveTotal)
	{
		Signature signature(obj["EntitySignatures"][j].GetString());
		p_ecs.SetEntitySignature(j, signature);
		for (ComponentType CompID = 0; CompID < p_ecs.GetTotalRegisteredComponents(); ++CompID)
		{
			if (p_ecs.GetEntitySignature(j).test(CompID))
			{
				if (p_ecs.GetComponentTypeName(CompID) == "Transform")
				{
					Transform transform;
					transform.SetComponentEntityID(j);

					if (transform.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Transform>(j, transform);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "RigidBody")
				{
					RigidBody rigidbody;
					rigidbody.SetComponentEntityID(j);
					if (rigidbody.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<RigidBody>(j, rigidbody);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Collider")
				{
					Collider collider;
					collider.SetComponentEntityID(j);
					if (collider.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Collider>(j, collider);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "NameTag")
				{
					NameTag nametag;
					nametag.SetComponentEntityID(j);
					if (nametag.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<NameTag>(j, nametag);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Sprite")
				{
					Sprite sprite;
					sprite.SetComponentEntityID(j);
					if (sprite.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Sprite>(j, sprite);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Logic")
				{
					Logic logic;
					logic.SetComponentEntityID(j);
					if (logic.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Logic>(j, logic);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Tag")
				{
					Tag tag;
					tag.SetComponentEntityID(j);

					if (tag.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Tag>(j, tag);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Audio")
				{
					Audio mAudio;
					mAudio.SetComponentEntityID(j);
					if (mAudio.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Audio>(j, mAudio);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "HUDComponent")
				{
					HUDComponent mHUD;
					mHUD.SetComponentEntityID(j);
					if (mHUD.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<HUDComponent>(j, mHUD);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Draggable")
				{
					Draggable draggable;
					draggable.SetComponentEntityID(j);

					if (draggable.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Draggable>(j, draggable);
					}
				}
				if (p_ecs.GetComponentTypeName(CompID) == "Text")
				{
					Text text;
					text.SetComponentEntityID(j);
					if (text.Deserialize(obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj()))
					{
						p_ecs.AddComponent<Text>(j, text);
					}
				}
				if (gcr&&gcr->contains(p_ecs.GetComponentTypeName(CompID)))
				{
					gcr->at(p_ecs.GetComponentTypeName(CompID)).deserializeAndAddFunc(j, obj["Components"][std::to_string(j).c_str()][static_cast<int>(CompID)].GetObj());
				}

			}
		}
		++j;
	}
	//std::cout << "Deserialize done\n";
	return true;
}

/*!*************************************************************************
NewSerialize serializes the scene into a hopefully more readable json file
than the old serialize did
****************************************************************************/
bool NocturnalEngine::SceneManager::NewSerialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const{

	writer->StartObject();
	//layer draw order
	writer->Key("LayerDrawOrder");
	writer->StartArray();
	for (int i = 0; i < NocturnalEngine::Graphic::MAX_LAYERS; ++i)
		writer->Int(NocturnalEngine::Graphic::layerDrawOrder[i]);
	writer->EndArray();

	//Begin entity array
	writer->Key("Entities");
	writer->StartArray();

	//if no entities, just end array and return
	if (p_ecs.GetAliveEntities() == 0) {
		writer->EndArray();
		writer->EndObject();
		return true;
	}

	std::set<Entity> aliveTotal =p_ecs.GetAliveEntitiesSet();
	PrefabManager& prefabs{ p_PrefabManager };
	for (Entity j:aliveTotal)
	{
		// Only serialize root entities — children are handled recursively
		if (p_Heirarchy.IsChild(j)) continue;
		SerializeEntity(writer, j, prefabs);
	
	}
	
	writer->EndArray(); //entity array

	writer->EndObject();
	return true;

}

void NocturnalEngine::SceneManager::SerializeEntity(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, Entity j, PrefabManager& prefabs) const {

	//Skip entities with no components
	if (!p_ecs.GetEntitySignature(j).any())
		return;
	//Start entity object
	writer->StartObject();
	//ID, not actually used when deserializing but for users to see better
	writer->Key("ID");
	writer->Int(static_cast<int>(j));

	bool isFrmPrefab{ false };
	PrefabManager::PrefabInfo pI{};
	if (prefabs.IsFromPrefab(j))
	{
		pI = prefabs.GetPrefabInfo(j);
		writer->Key("Prefab");
		writer->String(pI.prefabName.c_str());
		isFrmPrefab = true;
	}

	//Start Component array
	writer->Key("Components");
	writer->StartArray();

	//Engine components
	for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> comp : p_ECompReg.GetAll())
	{
		if (comp.second.hasFunc(j))
		{
			if (isFrmPrefab && comp.second.compareFunc(pI.prefabID, j)) continue;
			writer->StartObject();
			//component name always comes first,
			writer->Key("Component");
			writer->String(comp.first.c_str());
			// then start writing down the component
			comp.second.serializeFunc(j, writer);
			writer->EndObject();

		}
	}

	//Game components
	if (ComponentRegistry* meow = ServiceLocator::Get().GetGameComponents())
	{
		for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> comp : meow->GetAll())
		{
			if (comp.second.hasFunc(j))
			{
				if (isFrmPrefab && comp.second.compareFunc(pI.prefabID, j)) continue;
				writer->StartObject();
				writer->Key("Component");
				writer->String(comp.first.c_str());
				comp.second.serializeFunc(j, writer);
				writer->EndObject();

			}
		}
	}

	writer->EndArray(); //components

	if (p_Heirarchy.IsParent(j))
	{
		//Start Children array
		writer->Key("Children");
		writer->StartArray();
		const std::vector<Entity> children{ p_Heirarchy.GetChildren(j) };
		for (const Entity e:children)
		{
			SerializeEntity(writer, e, prefabs);
		}
		writer->EndArray();
	}

	writer->EndObject(); //entity
}
