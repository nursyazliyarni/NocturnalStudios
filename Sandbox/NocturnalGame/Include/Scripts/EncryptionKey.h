/*!------------------------------------------------------------------------------------------------
    File: EncryptionKey.h
    Project: NocturnalStudios

    Brief:
        Handles the hiding and revealing of encryption keyss

    Primary Author:
         Nur Syazliyarni (100%)
        - Created logic script for keys
        - Extended key logic for all animal scenes with stable scene-based key tracking.
        - Added level-gated key interaction rules.

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "ECS/ECS.h"
#include "Input/Input.h"
#include "Math/Vmath.h"
#include "Scripts/IScript.h"
#include <random>
#include <unordered_map>
#include <string>
#include <unordered_set>

namespace NocturnalEngine
{
    // Handles the "hidden key" mini-loop:
    // 1) key is hidden behind a random bush
    // 2) first click collects key into shared array (max 4)
    // 3) collected key reveals in front briefly, then hides again
    class EncryptionKey : public IScript
    {
    public:
        EncryptionKey();
        void Start() override;
        void Update(float dt) override;
        void End() override {}

        bool Requirements(Entity e) override {
            return p_ecs.HaveComponent<Transform>(e) && p_ecs.HaveComponent<Sprite>(e);
        }

        virtual std::string GetScriptName() override { return "encryptionkey"; }
        static size_t GetCollectedCount() { return sCollectedKeys.size(); }
        static size_t GetMaxKeys() { return kMaxKeys; }
        static void ResetRunState();

        virtual EncryptionKey* Clone() const override
        {
            return new EncryptionKey(*this);
        }
        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        // Finds all bush entities used as hiding spots.
        void FindHidingSpots();
        // Picks one random valid point inside a bush collider.
        vec2D GetRandomPointInBush(Entity bushEntity) const;
        // Checks click hit on visible key.
        bool IsMouseOverKey();
        // Returns whether this scene key is interactable at the current level.
        bool IsKeyInteractableForCurrentLevel() const;
        // Adds key to shared collection and starts the reveal window.
        void CollectKey();
        // Builds a stable scene id used for per-run letter assignment.
        std::string BuildSceneKey() const;
        // Assigns one unique encryption letter texture to each animal scene for this run.
        static void EnsureSceneAssignments();
        // Returns assigned letter texture key for the current scene.
        std::string ResolveAssignedTextureKey() const;
        // Checks whether this key id is already in collected list.
        bool IsCollected(const std::string& keyId) const;

    private:
        InputSystem* mInput{ nullptr };
        // Bush entities used to hide the key.
        std::vector<Entity> mHidingSpots{};
        bool mHasHidingSpots{ false };
        // Runtime key state flags.
        bool mCollected{ false };
        bool mRevealActive{ false };
        // Hidden location where key is placed behind a bush.
        vec2D mHiddenPos{};
        // Scene-stable id for per-run scene assignment.
        std::string mSceneKeyId{};
        // Letter-stable id for cross-scene key tracking.
        std::string mCollectionKeyId{};
        // Original state to restore during the brief reveal.
        vec2D mOriginalScale{};
        vec2D mOriginalPos{};
        int mOriginalLayer{ 0 };
        std::mt19937 mRng{ std::random_device{}() };
        float mRevealTimer{ 0.0f };
        float mRevealDuration{ 2.0f };

        // Shared across all EncryptionKey instances.
        // Acts like the player's collected key inventory.
        static inline std::unordered_set<std::string> sCollectedKeys{};
        // Per-run scene -> letter texture assignment.
        static inline std::unordered_map<std::string, std::string> sSceneAssignments{};
        // Hard cap requested: player can collect at most 4 keys.
        static constexpr size_t kMaxKeys = 4;
    };
}
