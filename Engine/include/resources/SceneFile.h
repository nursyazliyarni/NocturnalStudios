/*!------------------------------------------------------------------------------------------------
    File: SceneFile.h
    Project: NocturnalStudio

    This file contains SceneFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- SceneFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Resource.h"
#include "ECS/ECS_Def.h"

/**
 * @brief Resource wrapper for a scene file.
 *
 * SceneFile represents a scene resource loaded by the engine. It stores the
 * associated SceneID, whether this scene should be started with the game,
 * and whether the scene's preload step has completed.
 */
class SceneFile : public Resource {
public:
    /**
     * @brief Construct a new SceneFile.
     * @param id    ResourceID assigned to this resource.
     * @param key   Human-readable key/name for the resource.
     * @param filePath Filesystem path to the scene file.
     * @param startWithThisScene If true, the engine should start with this scene.
     */
    SceneFile(ResourceID id, const std::string& key, const std::string& filePath, bool startWithThisScene);

    /**
     * @brief Get the SceneID associated with this SceneFile.
     * @return const SceneID& A const reference to the stored SceneID.
     *
     * Use this to retrieve the unique identifier used by the ECS to reference the scene.
     */
    SceneID const& getSceneID() const;

    /**
     * @brief Set the SceneID for this SceneFile.
     * @param id The SceneID to associate with this scene resource.
     *
     * This is typically called when the scene is registered with the scene manager / ECS.
     */
    void setSceneId(const SceneID& id);

    /**
     * @brief Check whether the engine should start with this scene.
     * @return true if the engine is configured to start with this scene.
     */
    bool getStartWithThisScene() const;

    /**
     * @brief Mark the scene as having completed its preload step.
     *
     * Call this after any asynchronous or staged preload operations have finished.
     */
    void finishPreload();

    /**
     * @brief Check whether the scene has finished preloading.
     * @return true if preload has completed.
     */
    bool checkIsPreloaded() const;

private:
    /// Whether the engine should start with this scene.
    bool startWithThisScene;

    /// The SceneID assigned to this scene resource.
    SceneID sceneID;

    /// True once finishPreload() has been called.
    bool isPreloaded;
};
