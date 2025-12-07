/*!------------------------------------------------------------------------------------------------
    File: Audio.cpp
    Project: NocturnalStudio

    This file contains Audio resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Audio resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <iostream>

#include "resources/Audio.h"
#include "core/Logger.h"
extern Logger logger;

/**
 * @brief Construct a new Audio object and log creation.
 *
 * The constructor forwards parameters to the base Resource and logs the
 * creation event. Actual audio loading/initialization is handled elsewhere
 * (audio system/backend).
 *
 * @param id Resource identifier.
 * @param key Lookup key for the resource.
 * @param filePath Path to the audio file.
 */
Audio::Audio(ResourceID id, const std::string& key, const std::string& filePath)
    : Resource(id, key, filePath) {
    logger.log("Audio resource created: " + key + " from " + filePath);
}