/*!------------------------------------------------------------------------------------------------
    File: Random.h
    Project: NocturnalStudio

    Brief:
        Simple helppers for generating random values

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the Random.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/



#include "pch.h"
#include <random>

namespace NocturnalEngine
{
    namespace Random
    {
        //singleTON
        inline std::mt19937& Engine()
        {
            static std::mt19937 engine(std::random_device{}());
            return engine;
        }

        // random range its max inclusive for int but max exclusive for floats
        template<typename T>
        inline T Range(T min, T max)
        {
            if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(Engine());
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::uniform_real_distribution<T> dist(min, max);
                return dist(Engine());
            }
            else
            {
                static_assert(std::is_arithmetic_v<T>, "Random::Range requires numeric type");
            }
        }
    }
}