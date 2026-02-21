/*!------------------------------------------------------------------------------------------------
    File: pch.h
    Project: NocturnalStudio

	Brief:
        This file contains the precompiled header for the Nocturnal Engine.
        It includes standard library headers and commonly used engine headers
        to improve compilation times across the project.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the pch.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
//precompile header

#define _USE_MATH_DEFINES
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <array>
#include <bitset>
#include <cstdlib>
#include <random>
#include <math.h>

#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cassert>
#include <any>
#include <set>

#include <chrono>

#include "Core/Log.h" 
#include "Serialization/JSONincludes.h"

// Memory Manager defines
constexpr std::size_t kDefaultBlockSize = 256u;
constexpr std::size_t kDefaultBlocksPerPage = 128u;
