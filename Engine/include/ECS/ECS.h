#ifndef NOCTURNAL_ECS_H
#define NOCTURNAL_ECS_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <cassert>
#include "ECS_Def.h"
#include "ECS_Component.h"
#include "ECS_ComponentPool.h"

class System; // forward from Engine/include/System/System.h

namespace ecs
{
    // Forward
    class Registry;

    /**
     * @brief Simple System base interface used by the lightweight ECS runner.
     *
     * Derive from `ecs::ISystem` when implementing systems that operate on the
     * `Registry`. The `update` method is called by `Registry::update`.
     *
     * Example:
     * ```
     * struct MoveSystem : ecs::ISystem {
     *     void update(ecs::Registry& r, double dt) override {
     *         r.view<Transform, Velocity>([dt](ecs::Entity e, Transform& t, Velocity& v){
     *             t.x += v.vx * static_cast<float>(dt);
     *             t.y += v.vy * static_cast<float>(dt);
     *         });
     *     }
     * };
     * ```
     */
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        /**
         * @brief Called once per frame (or tick) by `Registry::update`.
         *
         * @param reg Reference to the registry so the system can query or modify entities/components.
         * @param dt  Delta time in seconds.
         */
        virtual void update(Registry& reg, double dt) = 0;
    };

    /**
     * @brief Type-erased interface for component pools.
     *
     * Implemented by `TypedPool<T>` to allow `Registry` to store heterogeneous
     * pools in a single container keyed by component type id.
     */
    struct IPool
    {
        virtual ~IPool() = default;

        /**
         * @brief Remove any component associated with entity `e` in this pool.
         *
         * Implementations must be safe to call even if `e` is not present.
         */
        virtual void erase(Entity e) = 0;

        /**
         * @brief Return the dense list of entities that currently own a component in this pool.
         *
         * The returned vector's order corresponds 1:1 with the pool's internal dense component vector.
         */
        virtual const std::vector<Entity>& entities() const = 0;
    };

    /**
     * @brief Concrete IPool implementation that holds a `ComponentPool<T>`.
     *
     * This wraps the template `ComponentPool<T>` and exposes the required
     * type-erased `IPool` functions.
     */
    template<typename T>
    struct TypedPool final : IPool
    {
        ComponentPool<T> pool;

        void erase(Entity e) override { pool.erase(e); }
        const std::vector<Entity>& entities() const override { return pool.entities(); }
    };

    /**
     * @brief Registry: manages entities, component pools and lightweight systems.
     *
     * Responsibilities:
     * - Create/destroy entities using a packed index+generation `Entity`.
     * - Create and provide access to per-component `ComponentPool<T>` instances.
     * - Provide view-style iteration over entities that own a set of components.
     * - Optionally run `ISystem` instances via `update`.
     *
     * Example usage:
     * ```
     * ecs::Registry reg;
     * auto e = reg.create();
     * reg.emplace<Transform>(e, 0.0f, 0.0f); // construct Transform for entity
     * reg.emplace<Velocity>(e, 1.0f, 0.0f);
     *
     * reg.view<Transform, Velocity>([](ecs::Entity ent, Transform& t, Velocity& v){
     *     t.x += v.vx;
     * });
     * ```
     */
    class Registry
    {
    public:
        Registry() = default;
        ~Registry() = default;

        // Entity lifecycle

        /**
         * @brief Create a new entity.
         *
         * Reuses previously freed indices when possible. The returned `Entity`
         * packs an index and generation. Newly created entities are marked alive.
         *
         * Usage:
         * ```
         * ecs::Entity e = reg.create();
         * ```
         *
         * @return A packed `Entity` id.
         */
        Entity create()
        {
            Entity e;
            if (!freeIndices.empty())
            {
                auto idx = freeIndices.back();
                freeIndices.pop_back();
                auto gen = generations[idx];
                e = make_entity(idx, gen);
            }
            else
            {
                std::uint32_t idx = static_cast<std::uint32_t>(generations.size());
                generations.push_back(0);
                e = make_entity(idx, 0);
            }
            alive.insert(e);
            return e;
        }

        /**
         * @brief Check whether an entity id refers to a currently alive entity.
         *
         * Validates:
         * - Not equal to `InvalidEntity`,
         * - Index is within known generations,
         * - Stored generation matches the current generation for that index,
         * - Entity is present in the `alive` set.
         *
         * Usage:
         * ```
         * if (reg.valid(e)) { // safe to use e // }
         * ```
         *
         * @param e Entity to validate.
         * @return true if `e` is valid and alive.
         */
        bool valid(Entity e) const
        {
            if (e == InvalidEntity) return false;
            auto idx = entity_index(e);
            if (idx >= generations.size()) return false;
            return entity_generation(e) == generations[idx] && alive.count(e) > 0;
        }

        /**
         * @brief Destroy an entity and remove all its components.
         *
         * - Removes the entity from every registered component pool.
         * - Marks it not alive and increments the generation for its index.
         * - Adds the freed index to the free list for reuse.
         *
         * If `e` is not valid this is a no-op.
         *
         * Usage:
         * ```
         * reg.destroy(e);
         * ```
         *
         * @param e Entity to destroy.
         */
        void destroy(Entity e)
        {
            if (!valid(e)) return;
            // remove from all component pools
            for (auto& [typeId, ipool] : pools)
            {
                if (ipool) ipool->erase(e);
            }
            alive.erase(e);
            auto idx = entity_index(e);
            generations[idx]++;
            freeIndices.push_back(idx);
        }

        // Components API

        /**
         * @brief Emplace a component `T` for entity `e` (construct in pool).
         *
         * Asserts that `e` is valid. If a `T` already exists for `e`, it will be
         * replaced (the underlying `ComponentPool<T>::emplace` handles replacement).
         *
         * Usage:
         * ```
         * reg.emplace<Transform>(e, 1.0f, 2.0f); // forwards to Transform ctor
         * ```
         *
         * @tparam T Component type.
         * @tparam Args Constructor argument types for `T`.
         * @param e Entity to attach component to.
         * @param args Arguments forwarded to `T`'s constructor.
         * @return Reference to the placed component.
         */
        template<typename T, typename... Args>
        T& emplace(Entity e, Args&&... args)
        {
            assert(valid(e) && "Emplacing to invalid entity");
            auto& pool = pool_for<T>();
            auto& comp = pool.emplace(e, std::forward<Args>(args)...);
            return comp;
        }

        /**
         * @brief Check whether entity `e` has component `T`.
         *
         * Returns false if no pool for `T` exists or the pool does not contain `e`.
         *
         * Usage:
         * ```
         * if (reg.has<Health>(e)) { // use reg.get<Health>(e) // }
         * ```
         *
         * @tparam T Component type.
         * @param e Entity to query.
         * @return true if `e` has a `T` component.
         */
        template<typename T>
        bool has(Entity e) const
        {
            auto p = try_pool<T>();
            return p ? p->has(e) : false;
        }

        /**
         * @brief Get a mutable pointer to component `T` for `e`, or nullptr.
         *
         * Usage:
         * ```
         * if (auto* h = reg.get<Health>(e)) { h->hp -= 1; }
         * ```
         *
         * @tparam T Component type.
         * @param e Entity to query.
         * @return Pointer to `T` or `nullptr` if absent.
         */
        template<typename T>
        T* get(Entity e)
        {
            auto p = try_pool<T>();
            return p ? p->get(e) : nullptr;
        }

        /**
         * @brief Const overload of `get`.
         *
         * @tparam T Component type.
         * @param e Entity to query.
         * @return Const pointer to `T` or `nullptr`.
         */
        template<typename T>
        const T* get(Entity e) const
        {
            auto p = try_pool<T>();
            return p ? p->get(e) : nullptr;
        }

        /**
         * @brief Remove component `T` from entity `e` if present.
         *
         * Safe to call even if no pool for `T` exists.
         *
         * Usage:
         * ```
         * reg.remove<Inventory>(e);
         * ```
         *
         * @tparam T Component type.
         * @param e Entity to modify.
         */
        template<typename T>
        void remove(Entity e)
        {
            auto p = try_pool<T>();
            if (p) p->erase(e);
        }

        /**
         * @brief Iterate over entities that have all components Ts and call `f`.
         *
         * This implements a simple view: it selects the smallest involved component
         * pool to iterate and checks presence of the remaining components via `has`.
         * The callable `f` is invoked with signature `f(Entity, Ts&...)`.
         *
         * Usage:
         * ```
         * reg.view<Transform, Renderable>([](ecs::Entity e, Transform& t, Renderable& r){
         *     // process entity
         * });
         * ```
         *
         * @tparam Ts Component types to require.
         * @tparam Func Callable type accepting `(Entity, Ts&...)`.
         * @param f Callable invoked for each matching entity.
         */
        template<typename... Ts, typename Func>
        void view(Func&& f)
        {
            auto* smallest = smallest_pool<Ts...>();
            if (!smallest) return;
            auto& entitiesRef = smallest->entities();
            for (auto e : entitiesRef)
            {
                if ((has<Ts>(e) && ...))
                {
                    f(e, *get<Ts>(e)...);
                }
            }
        }

        // Systems (optional lightweight runner)

        /**
         * @brief Create and add a system instance derived from `ISystem`.
         *
         * Returns a reference to the newly added system. Systems are owned by
         * the registry and updated via `update`.
         *
         * Usage:
         * ```
         * auto& sys = reg.addSystem<MoveSystem>(/* ctor args *);
         * ```
         *
         * @tparam T System type deriving from `ISystem`.
         * @tparam Args Constructor argument types for `T`.
         * @param args Forwarded to `T`'s constructor.
         * @return Reference to the added system.
         */
        template<typename T, typename... Args>
        T& addSystem(Args&&... args)
        {
            static_assert(std::is_base_of<ISystem, T>::value, "System must derive from ecs::ISystem");
            auto sys = std::make_unique<T>(std::forward<Args>(args)...);
            auto* ptr = sys.get();
            systems.emplace_back(std::move(sys));
            return *ptr;
        }

        /**
         * @brief Run `update` on all registered systems.
         *
         * Should be called once per frame (or tick).
         *
         * Usage:
         * ```
         * reg.update(deltaTime);
         * ```
         *
         * @param dt Delta time passed to each system.
         */
        void update(double dt)
        {
            for (auto& s : systems) s->update(*this, dt);
        }

    private:
        /**
         * @brief Get-or-create the concrete `ComponentPool<T>` for type `T`.
         *
         * Creates a new `TypedPool<T>` and inserts it into `pools` if missing.
         *
         * @tparam T Component type.
         * @return Reference to the `ComponentPool<T>`.
         */
        template<typename T>
        ComponentPool<T>& pool_for()
        {
            auto id = component_type_id<T>();
            auto it = pools.find(id);
            if (it == pools.end())
            {
                auto typed = std::make_unique<TypedPool<T>>();
                auto* p = &typed->pool;
                pools.emplace(id, std::move(typed));
                return *p;
            }
            auto* typed = static_cast<TypedPool<T>*>(it->second.get());
            return typed->pool;
        }

        /**
         * @brief Try to find an existing `ComponentPool<T>`; return nullptr if none.
         *
         * Const method returning a non-owning pointer suitable for querying presence
         * or retrieving components without forcing pool creation.
         *
         * @tparam T Component type.
         * @return Pointer to `ComponentPool<T>` or nullptr.
         */
        template<typename T>
        ComponentPool<T>* try_pool() const
        {
            auto it = pools.find(component_type_id<T>());
            if (it == pools.end()) return nullptr;
            auto* typed = static_cast<TypedPool<T>*>(it->second.get());
            return const_cast<ComponentPool<T>*>(&typed->pool);
        }

        /**
         * @brief Pick the smallest pool among Ts (used to minimize iteration).
         *
         * Returns the `IPool*` corresponding to the component type that currently
         * holds the fewest entities among the requested set. Returns nullptr if
         * none of the requested types have an existing pool.
         *
         * @tparam Ts Component types to consider.
         * @return Pointer to the smallest `IPool` or nullptr.
         */
        template<typename... Ts>
        IPool* smallest_pool() const
        {
            IPool* smallest = nullptr;
            std::size_t smallestSize = std::numeric_limits<std::size_t>::max();
            (void)std::initializer_list<int>{ (update_smallest<Ts>(smallest, smallestSize), 0)... };
            return smallest;
        }

        /**
         * @brief Helper: update `smallest` if pool for `T` is smaller than current.
         *
         * @tparam T Component type to evaluate.
         * @param smallest Reference to the current smallest IPool pointer (may be updated).
         * @param smallestSize Reference to the size of the current smallest pool.
         */
        template<typename T>
        void update_smallest(IPool*& smallest, std::size_t& smallestSize) const
        {
            auto it = pools.find(component_type_id<T>());
            if (it == pools.end()) return;
            auto sz = it->second->entities().size();
            if (sz < smallestSize)
            {
                smallest = it->second.get();
                smallestSize = sz;
            }
        }

    private:
        std::vector<std::unique_ptr<ISystem>> systems; ///< Owned systems updated by `update`.

        // entity storage
        std::unordered_set<Entity> alive;  ///< Set of currently alive entity ids.
        std::vector<Generation> generations; ///< Per-index generation counters.
        std::vector<std::uint32_t> freeIndices; ///< Freed indices available for reuse.

        // component pools keyed by runtime component type id
        std::unordered_map<std::size_t, std::unique_ptr<IPool>> pools;
    };
}

// Provide a project-level ECS class name expected by other systems.
class ECS : public ecs::Registry
{
public:
    using ecs::Registry::Registry;
};

#endif // NOCTURNAL_ECS_H

