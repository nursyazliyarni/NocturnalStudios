/*!------------------------------------------------------------------------------------------------
    File: EncryptionKey.h
    Project: NocturnalStudios

    Brief:
        Handles the hiding and revealing of encryption keyss

    Primary Author:
         Nur Syazliyarni (100%)
        - Created logic script for keys

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "ECS/ECS.h"
#include "Input/Input.h"
#include "Math/Vmath.h"
#include "Scripts/IScript.h"
#include <random>
#include <string>
#include <unordered_set>

namespace NocturnalEngine
{
    // Handles the "hidden key" mini-loop:
    // 1) key is hidden at random spot inside Environment bounds
    // 2) first click reveals key
    // 3) second click collects key into shared array (max 4) and hides it again
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

        virtual EncryptionKey* Clone() const override
        {
            return new EncryptionKey(*this);
        }
        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

    private:
        // Finds the scene object tagged as Environment.
        void FindEnvironment();
        // Picks one random valid point inside environment collider bounds.
        vec2D RandomPointInsideEnvironment();
        // Returns half-size of key for bound clamping (collider first, transform fallback).
        vec2D GetKeyHalfExtents();
        // Checks click hit on visible key.
        bool IsMouseOverKey();
        // Checks click proximity to hidden key location.
        bool IsMouseNearHiddenSpot() const;
        // Returns whether this scene key is interactable at the current level.
        bool IsKeyInteractableForCurrentLevel() const;
        // Adds key to shared collection and hides it.
        void CollectKey();
        // Builds a scene-stable collection id for this key.
        std::string BuildCollectionKey() const;
        // Checks whether this key id is already in collected list.
        bool IsCollected(const std::string& keyId) const;

    private:
        InputSystem* mInput{ nullptr };
        // Cached environment entity used for random placement.
        Entity mEnvironment{ 0 };
        bool mHasEnvironment{ false };
        // Runtime key state flags.
        bool mCollected{ false };
        bool mRevealed{ false };
        // Hidden location where key is placed while invisible.
        vec2D mHiddenPos{};
        // Scene-stable id for cross-scene key tracking.
        std::string mCollectionKeyId{};
        // Original scale to restore on reveal.
        vec2D mOriginalScale{};
        std::mt19937 mRng{ std::random_device{}() };
        // Radius around hidden location that counts as first "reveal" click.
        float mRevealRadius{ 0.08f };

        // Shared across all EncryptionKey instances.
        // Acts like the player's collected key inventory.
        static inline std::unordered_set<std::string> sCollectedKeys{};
        // Hard cap requested: player can collect at most 4 keys.
        static constexpr size_t kMaxKeys = 4;
    };
}
