#ifndef NOCTURNAL_ECS_COMPONENT_H
#define NOCTURNAL_ECS_COMPONENT_H

#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include "ECS_Def.h"

namespace ecs
{
    /**
     * @brief Optional base tag for components.
     *
     * Use when you want a polymorphic interface or to store heterogeneous
     * component instances as `IComponent*`. Not required for the component
     * ID system below, which is type-based.
     */
    struct IComponent { virtual ~IComponent() = default; };

    /**
     * @brief Map a runtime `std::type_index` to a compact size_t id.
     *
     * This function keeps an internal `static` map that assigns an incremental
     * id to each unique `std::type_index` the first time it is seen.
     *
     * Notes:
     * - Static data is initialized in a thread-safe manner in C++11 and later,
     *   but concurrent registrations (multiple threads calling this while
     *   inserting new types) may still race on the map itself. If you need
     *   lock-free multi-threaded registration, provide external synchronization.
     *
     * Usage:
     * ```
     * std::size_t id = ecs::component_type_id(std::type_index(typeid(MyComp)));
     * ```
     *
     * @param ti The type index to get an id for.
     * @return A unique id for that runtime type (0..N-1 as types are registered).
     */
    inline std::size_t component_type_id(std::type_index ti)
    {
        static std::unordered_map<std::type_index, std::size_t> ids;
        static std::size_t counter = 0;
        auto it = ids.find(ti);
        if (it != ids.end()) return it->second;
        auto id = counter++;
        ids.emplace(ti, id);
        return id;
    }

    /**
     * @brief Template helper to get a compile-time-friendly component id.
     *
     * Convenience wrapper around the `std::type_index` based registration. Call
     * as `ecs::component_type_id<MyComponent>()` to obtain a stable small id
     * for the given component type.
     *
     * Usage:
     * ```
     * struct Transform { float x,y; };
     * std::size_t tid = ecs::component_type_id<Transform>();
     * ```
     *
     * @tparam T Component type.
     * @return A unique id for `T`.
     */
    template<typename T>
    inline std::size_t component_type_id()
    {
        static const std::size_t id = component_type_id(std::type_index(typeid(T)));
        return id;
    }
}

#endif // NOCTURNAL_ECS_COMPONENT_H