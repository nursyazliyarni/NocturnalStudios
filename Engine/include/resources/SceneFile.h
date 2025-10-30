#pragma once

#include "Resource.h"
#include "ECS/ECS_Def.h"

class SceneFile : public Resource {
public:
	SceneFile(ResourceID id, const std::string& key, const std::string& filePath, bool startWithThisScene);
	SceneID const& getSceneID() const;
	void setSceneId(const SceneID& id);
	bool getStartWithThisScene() const;
	void finishPreload();
	bool checkIsPreloaded() const;

private:
	bool startWithThisScene;
	SceneID sceneID;
	bool isPreloaded;
};
