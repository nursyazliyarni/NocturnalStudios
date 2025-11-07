#include "resources/SceneFile.h"
#include "core/Logger.h"
extern Logger logger;

SceneFile::SceneFile(ResourceID id, const std::string& key, const std::string& filePath, bool startWithThisScene) :
	Resource(id, key, filePath),
	sceneID(DEFAULT_SCENE),
	startWithThisScene(startWithThisScene),
	isPreloaded(false) {
	logger.log("SceneFile created: " + key + " from " + filePath);
	logger.log("Start with this scene: " + std::to_string(startWithThisScene));
}

SceneID const& SceneFile::getSceneID() const {
	return sceneID;
}

void SceneFile::setSceneId(const SceneID& id_) {
	sceneID = id_;
}

bool SceneFile::getStartWithThisScene() const {
	return startWithThisScene;
}

void SceneFile::finishPreload() {
	isPreloaded = true;
}

bool SceneFile::checkIsPreloaded() const {
	return isPreloaded;
}
