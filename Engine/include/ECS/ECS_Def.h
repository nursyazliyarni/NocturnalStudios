#ifndef NOCTURNAL_ECS_DEF_H
#define NOCTURNAL_ECS_DEF_H

#include <cstdint>
#include <limits>
#include <typeindex>

namespace ecs
{
    /**
     * @file ECS_Def.h
     * @brief Small, packed entity representation and helpers.
     *
     * Entities are 32-bit values where:
     * - lower 24 bits = index (0 .. 0x00FFFFFF)
     * - upper 8 bits  = generation (0 .. 0xFF)
     *
     * This packing is compact and fast for small/medium projects.
     */

    /// Lightweight alias for an entity id (packed index+generation).
    using Entity = std::uint32_t;

    /// Alias for generation values (separate type to clarify intent).
    using Generation = std::uint32_t;

    /// A sentinel value representing an invalid or empty entity.
    constexpr Entity InvalidEntity = std::numeric_limits<Entity>::max();

    /**
     * @brief Create a packed `Entity` from its parts.
     *
     * Packs `index` into the lower 24 bits and `gen` into the upper 8 bits.
     *
     * Notes:
     * - Only the low 24 bits of `index` are used; values >= 2^24 will be truncated.
     * - Only the low 8 bits of `gen` are stored (generations > 255 will wrap).
     *
     * Usage:
     * ```
     * // valid index in [0, 0x00FFFFFF], generation in [0, 255]
     * ecs::Entity e = ecs::make_entity(42u, 1u);
     * ```
     *
     * @param index The entity index (stored in bits 0..23).
     * @param gen   The generation (stored in bits 24..31).
     * @return Packed `Entity`.
     */
    inline Entity make_entity(std::uint32_t index, std::uint32_t gen)
    {
        return static_cast<Entity>((gen << 24) | (index & 0x00FFFFFFu));
    }

    /**
     * @brief Extract the index component from a packed `Entity`.
     *
     * Usage:
     * ```
     * ecs::Entity e = ecs::make_entity(42u, 1u);
     * std::uint32_t idx = ecs::entity_index(e); // 42
     * ```
     *
     * @param e Packed `Entity`.
     * @return The index stored in the lower 24 bits.
     */
    inline std::uint32_t entity_index(Entity e) { return (e & 0x00FFFFFFu); }

    /**
     * @brief Extract the generation component from a packed `Entity`.
     *
     * Usage:
     * ```
     * ecs::Entity e = ecs::make_entity(42u, 1u);
     * std::uint32_t gen = ecs::entity_generation(e); // 1
     * ```
     *
     * @param e Packed `Entity`.
     * @return The generation stored in the upper 8 bits.
     */
    inline std::uint32_t entity_generation(Entity e) { return (e >> 24); }
}

#endif // NOCTURNAL_ECS_DEF_H