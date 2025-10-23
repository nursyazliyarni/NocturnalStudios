#ifndef NOCTURNAL_ECS_COMPONENT_POOL_H
#define NOCTURNAL_ECS_COMPONENT_POOL_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <type_traits>
#include "ECS_Def.h"

namespace ecs
{
    /**
     * @brief Sparse-set-style container for a single component type `T`.
     *
     * This container stores component `T` instances in a dense vector (`values`)
     * while keeping a mapping from `Entity` -> dense index. The `dense_entities`
     * vector holds the entity ids corresponding to the dense values.
     *
     * Complexity:
     * - `has` : average O(1)
     * - `emplace` : amortized O(1)
     * - `get` : average O(1)
     * - `erase` : average O(1) (swap-remove semantics)
     *
     * Usage:
     * ```
     * ecs::ComponentPool<Transform> pool;
     * ecs::Entity e = ecs::make_entity(1,0);
     * pool.emplace(e, /* ctor args for Transform *);
     * if (pool.has(e)) {
     *     Transform* t = pool.get(e);
     *     // use *t
     * }
     * pool.erase(e);
     * ```
     *
     * Note:
     * - `emplace` will replace the existing value if the entity already exists.
     *   The implementation assigns a newly constructed `T` into the existing slot,
     *   so `T` must be copy- or move-assignable for replacement to work.
     */
    template<typename T>
    class ComponentPool
    {
    public:
        using value_type = T;

        /**
         * @brief Check whether the pool contains a component for entity `e`.
         *
         * @param e Entity to check.
         * @return true if a component exists for `e`, false otherwise.
         */
        bool has(Entity e) const
        {
            auto idx = entity_to_dense_index.find(e);
            return idx != entity_to_dense_index.end();
        }

        /**
         * @brief Emplace a component for `e` constructed from `Args...`.
         *
         * If a component for `e` already exists, it will be replaced in-place
         * by assigning a fresh `T` constructed with `Args...`.
         *
         * @tparam Args Constructor argument types for `T`.
         * @param e Entity to associate with the component.
         * @param args Arguments forwarded to `T`'s constructor.
         * @return Reference to the emplaced (or replaced) component.
         *
         * Example:
         * ```
         * pool.emplace(e, 10.0f, 20.0f); // forwards to T(10.0f, 20.0f)
         * ```
         */
        template<typename... Args>
        T& emplace(Entity e, Args&&... args)
        {
            auto it = entity_to_dense_index.find(e);
            if (it != entity_to_dense_index.end())
            {
                // replace in-place
                values[it->second] = T(std::forward<Args>(args)...);
                return values[it->second];
            }
            std::size_t denseIdx = values.size();
            values.emplace_back(std::forward<Args>(args)...);
            dense_entities.push_back(e);
            entity_to_dense_index.emplace(e, denseIdx);
            return values.back();
        }

        /**
         * @brief Remove the component associated with `e`, if present.
         *
         * Removal uses swap-with-last to maintain dense storage:
         * - Move the last element into the removed slot (if not the last),
         * - Update the moved entity's index mapping,
         * - Pop the last element.
         *
         * If `e` is not present, this is a no-op.
         *
         * Usage:
         * ```
         * pool.erase(e);
         * ```
         *
         * @param e Entity to remove.
         */
        void erase(Entity e)
        {
            auto it = entity_to_dense_index.find(e);
            if (it == entity_to_dense_index.end()) return;
            std::size_t denseIdx = it->second;
            std::size_t last = values.size() - 1;
            if (denseIdx != last)
            {
                values[denseIdx] = std::move(values[last]);
                Entity movedE = dense_entities[last];
                dense_entities[denseIdx] = movedE;
                entity_to_dense_index[movedE] = denseIdx;
            }
            values.pop_back();
            dense_entities.pop_back();
            entity_to_dense_index.erase(it);
        }

        /**
         * @brief Get a pointer to the component for `e`, or `nullptr` if absent.
         *
         * @param e Entity to query.
         * @return Pointer to `T` if found, otherwise `nullptr`.
         */
        T* get(Entity e)
        {
            auto it = entity_to_dense_index.find(e);
            if (it == entity_to_dense_index.end()) return nullptr;
            return &values[it->second];
        }

        /**
         * @brief Const overload of `get`.
         *
         * @param e Entity to query.
         * @return Const pointer to `T` if found, otherwise `nullptr`.
         */
        const T* get(Entity e) const
        {
            auto it = entity_to_dense_index.find(e);
            if (it == entity_to_dense_index.end()) return nullptr;
            return &values[it->second];
        }

        /**
         * @brief Access the dense list of entities currently owning components.
         *
         * The indexes of `dense_entities` correspond 1:1 with `data()` vector.
         *
         * @return const reference to the vector of entities.
         */
        const std::vector<Entity>& entities() const { return dense_entities; }

        /**
         * @brief Mutable access to the dense component storage.
         *
         * @return Reference to the vector holding component `T` instances.
         */
        std::vector<T>& data() { return values; }

        /**
         * @brief Const access to the dense component storage.
         *
         * @return Const reference to the vector holding component `T` instances.
         */
        const std::vector<T>& data() const { return values; }

        /**
         * @brief Remove all components and reset internal mapping.
         *
         * Usage:
         * ```
         * pool.clear();
         * ```
         */
        void clear()
        {
            values.clear();
            dense_entities.clear();
            entity_to_dense_index.clear();
        }

    private:
        std::vector<T> values;                             ///< Dense component storage.
        std::vector<Entity> dense_entities;                ///< Parallel dense list of owning entities.
        std::unordered_map<Entity, std::size_t> entity_to_dense_index; ///< Sparse mapping.
    };
}

#endif // NOCTURNAL_ECS_COMPONENT_POOL_H
