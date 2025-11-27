/*!------------------------------------------------------------------------------------------------
    File: Audio.h
    Project: NocturnalStudio

    This file contains Audio resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Audio resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "resources/Resource.h"
#include <string>

/**
 * @brief Lightweight wrapper for audio resources managed by the engine.
 *
 * Audio represents a loaded audio asset (e.g. sound effect or music) and
 * inherits common behavior from Resource.
 */
class Audio : public Resource {
public:
    /**
     * @brief Construct a new Audio resource.
     *
     * Registers a new Audio resource with the given ResourceID, identifies it
     * with a key, and stores the original file path the audio was loaded from.
     *
     * @param id Unique identifier assigned to this resource.
     * @param key Human-readable key used to look up this resource.
     * @param filePath Filesystem path to the audio asset.
     */
    Audio(ResourceID id, const std::string& key, const std::string& filePath);

    /**
     * @brief Copying is not allowed for resources.
     *
     * Audio resources are not copyable to avoid duplicating underlying
     * platform/audio-system handles and to ensure single ownership semantics.
     */
    Audio(Audio const& other) = delete;

    /**
     * @brief Copy-assignment disabled.
     */
    Audio& operator=(Audio const& other) = delete;

    /**
     * @brief Move constructor allowed to transfer ownership.
     */
    Audio(Audio&& other) = default;

    /**
     * @brief Move-assignment allowed to transfer ownership.
     */
    Audio& operator=(Audio&& other) = default;
};