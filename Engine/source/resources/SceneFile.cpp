/*!------------------------------------------------------------------------------------------------
    File: SceneFile.cpp
    Project: NocturnalStudio

    This file contains SceneFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- SceneFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "resources/SceneFile.h"
#include "core/Logger.h"
extern Logger logger;

/**
 * @brief Construct a new SceneFile.
 * 
 * The constructor initializes the SceneFile resource with the given parameters
 * and sets default values for sceneID and isPreloaded. It also logs the creation
 * of the SceneFile.
 * 
 * @param id    ResourceID assigned to this resource.
 * @param key   Human-readable key/name for the resource.
 * @param filePath Filesystem path to the scene file.
 * @param startWithThisScene If true, the engine should start with this scene.
 */
SceneFile::SceneFile(ResourceID id, const std::string& key, const std::string& filePath, bool startWithThisScene) :
	Resource(id, key, filePath),
	sceneID(DEFAULT_SCENE),
	startWithThisScene(startWithThisScene),
	isPreloaded(false) {
	logger.log("SceneFile created: " + key + " from " + filePath);
	logger.log("Start with this scene: " + std::to_string(startWithThisScene));
}

/**
 * @brief Get the SceneID associated with this SceneFile.
 * 
 * @return SceneID const& 
 */
SceneID const& SceneFile::getSceneID() const {
	return sceneID;
}

/**
 * @brief Set the SceneID for this SceneFile.
 * 
 * @param id The SceneID to associate with this scene resource.
 */
void SceneFile::setSceneId(const SceneID& id_) {
	sceneID = id_;
}

/**
 * @brief Check whether the engine should start with this scene.
 * @return true if the engine is configured to start with this scene.
 */
bool SceneFile::getStartWithThisScene() const {
	return startWithThisScene;
}

/**
 * @brief Mark the scene as having completed its preload step.
 *
 * Call this after any asynchronous or staged preload operations have finished.
 */
void SceneFile::finishPreload() {
	isPreloaded = true;
}

/**
 * @brief Check whether the scene has finished preloading.
 * @return true if preload has completed.
 */
bool SceneFile::checkIsPreloaded() const {
	return isPreloaded;
}
