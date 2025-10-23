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

void SceneManager::InitScene(SceneID id){

	for (Scene s:scenes)
	{
		if (s.id==id)
		{
			curr_scene = std::make_unique<Scene>(s);
		}
	}
}

void SceneManager::Update(){
	if (curr_scene)
	{
		for (LayerID l : curr_scene->layers)
		{
			//TODO: to add logic & Physics code in here once implemented
		}
	}
	
}
void SceneManager::Render(){
	if (curr_scene)
	{
		for (LayerID l : curr_scene->layers)
		{
			//TODO: to add rendering  code in here once implemented
		}
	}
}

SceneManager::SceneManager() :scenes{/*will be loaded from resouce manger perhaps? either thatt*/} {
	ECS& ecs = ECS::getECS();
}
