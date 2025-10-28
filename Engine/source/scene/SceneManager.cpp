/* !------------------------------------------------------------------------------------------------
	File: LevelManager.cpp
	Project : NocturnalStudio

	This file contains ...

	Primary Author :
		Name(%)
		- Brief

	Secondary Author :
		Name(%)
		- Brief

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "./scene/SceneManager.h"

void SceneManager::InitScene(SceneID id) {


}

void SceneManager::Update() {
	if (curr_scene)
	{
		for (LayerID l : curr_scene->layers)
		{
			//TODO: to add Physics, and then logic code in here once implemented
		}
	}

}
void SceneManager::Render() {
	if (curr_scene)
	{
		for (LayerID l : curr_scene->layers)
		{
			//TODO: to add rendering  code in here once implemented
		}
	}
}

void SceneManager::UnloadAllEntities() {
	ECS::getECS().emptyAllEntities();
}
void SceneManager::LoadAllEntities() {

}

SceneManager::SceneManager() {
	ECS& ecs = ECS::getECS(); //this is just to test that ECS works
}
