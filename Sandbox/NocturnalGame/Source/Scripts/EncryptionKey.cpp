/*!------------------------------------------------------------------------------------------------
    File: EncryptionKey.cpp
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
#include "pch.h"
#include "Scripts/EncryptionKey.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "GUI/GUI.h"
#include "Scripts/GlobalGameManager.h"
#include "Math/Random.h"
#include <algorithm>
#include <cctype>
#include <array>
#include <random>

namespace NocturnalEngine
{
    namespace
    {
        constexpr std::array<const char*, 4> kSceneIds{
            "scene:flamingobtn",
            "scene:sealbtn",
            "scene:horsebtn",
            "scene:owlbtn"
        };

        constexpr std::array<const char*, 4> kEncryptionTextureKeys{
            "Play/encryption_h",
            "Play/encryption_e",
            "Play/encryption_l",
            "Play/encryption_p"
        };
    }

    // Cache input singleton.
    EncryptionKey::EncryptionKey()
        : mInput{ InputSystem::GetInstance().get() }
    {
    }

    // Initialize key state when scene starts.
    void EncryptionKey::Start()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        auto& keyTransform = p_ecs.GetComponent<Transform>(entityID);
        mOriginalScale = keyTransform.GetScale();
        mOriginalPos = keyTransform.GetPos();
        if (p_ecs.HaveComponent<Sprite>(entityID))
            mOriginalLayer = p_ecs.GetComponent<Sprite>(entityID).LayerOrder;

        FindHidingSpots();
        if (!mHasHidingSpots)
            return;

        mSceneKeyId = BuildSceneKey();
        EnsureSceneAssignments();
        if (p_ecs.HaveComponent<Sprite>(entityID))
        {
            p_ecs.GetComponent<Sprite>(entityID).SetTexture(ResolveAssignedTextureKey());
        }

        mCollectionKeyId = ResolveAssignedTextureKey();
        mCollected = IsCollected(mCollectionKeyId);
        mRevealActive = false;
        mRevealTimer = 0.0f;

        if (mCollected)
        {
            keyTransform.SetScale(0.0f, 0.0f);
            return;
        }

        const int hideIndex = Random::Range(0, static_cast<int>(mHidingSpots.size()) - 1);
        const Entity bushEntity = mHidingSpots[static_cast<size_t>(hideIndex)];
        mHiddenPos = GetRandomPointInBush(bushEntity);
        keyTransform.SetPos(mHiddenPos.x, mHiddenPos.y);

        if (p_ecs.HaveComponent<Sprite>(entityID) && p_ecs.HaveComponent<Sprite>(bushEntity))
        {
            auto& keySprite = p_ecs.GetComponent<Sprite>(entityID);
            auto& bushSprite = p_ecs.GetComponent<Sprite>(bushEntity);
            keySprite.SetLayerOrder(std::max(1, bushSprite.LayerOrder - 1));
        }
    }

    void EncryptionKey::Update(float dt)
    {
        if (!mHasHidingSpots)
            return;
        if (!IsKeyInteractableForCurrentLevel())
            return;

        if (mRevealActive)
        {
            mRevealTimer -= dt;
            if (mRevealTimer <= 0.0f && p_ecs.HaveComponent<Transform>(entityID))
            {
                p_ecs.GetComponent<Transform>(entityID).SetScale(0.0f, 0.0f);
                mRevealActive = false;
            }
            return;
        }

        if (mCollected || !mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
            return;

        if (IsMouseOverKey())
        {
            CollectKey();
        }
    }

    void EncryptionKey::FindHidingSpots()
    {
        mHidingSpots.clear();
        mHasHidingSpots = false;

        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) ||
                !p_ecs.HaveComponent<Transform>(e) ||
                !p_ecs.HaveComponent<Collider>(e) ||
                !p_ecs.HaveComponent<Sprite>(e))
                continue;

            if (p_ecs.GetComponent<NameTag>(e).GetNameTag() == "Bush")
            {
                mHidingSpots.push_back(e);
            }
        }

        mHasHidingSpots = !mHidingSpots.empty();
    }

    vec2D EncryptionKey::GetRandomPointInBush(Entity bushEntity) const
    {
        if (!p_ecs.HaveComponent<Transform>(bushEntity) || !p_ecs.HaveComponent<Collider>(bushEntity))
            return p_ecs.GetComponent<Transform>(entityID).GetPos();

        const Transform& bushTransform{ p_ecs.GetComponent<Transform>(bushEntity) };
        const Collider& bushCollider{ p_ecs.GetComponent<Collider>(bushEntity) };

        for (int i = 0; i < 2; ++i)
        {
            if (bushCollider[i].mCol != Collider::ColliderType::rect &&
                bushCollider[i].mCol != Collider::ColliderType::box)
            {
                continue;
            }

            const vec2D offsetPos{
                bushTransform.GetPos().x + bushCollider[i].mOffset.x,
                bushTransform.GetPos().y + bushCollider[i].mOffset.y
            };
            const vec2D minPt{
                offsetPos.x - bushCollider[i].mMin.x,
                offsetPos.y - bushCollider[i].mMin.y
            };
            const vec2D maxPt{
                offsetPos.x + bushCollider[i].mMax.x,
                offsetPos.y + bushCollider[i].mMax.y
            };

            const float insetX = std::max(0.0f, (maxPt.x - minPt.x) * 0.15f);
            const float insetY = std::max(0.0f, (maxPt.y - minPt.y) * 0.15f);

            return {
                Random::Range(minPt.x + insetX, maxPt.x - insetX),
                Random::Range(minPt.y + insetY, maxPt.y - insetY)
            };
        }

        return bushTransform.GetPos();
    }

    // Hit test used only after key is revealed.
    bool EncryptionKey::IsMouseOverKey()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return false;

        vec2D mousePos{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        auto& transform = p_ecs.GetComponent<Transform>(entityID);

        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& collider = p_ecs.GetComponent<Collider>(entityID);
            for (int i = 0; i < 2; ++i)
            {
                vec2D offsetPos{ transform.GetPos().x + collider[i].mOffset.x,
                                 transform.GetPos().y + collider[i].mOffset.y };

                switch (collider[i].mCol)
                {
                case Collider::ColliderType::circle:
                case Collider::ColliderType::bubble:
                case Collider::ColliderType::bossball:
                    if (distance(mousePos, offsetPos) < collider[i].mRadius) return true;
                    break;
                case Collider::ColliderType::rect:
                case Collider::ColliderType::box:
                {
                    const float halfWidth = std::max(std::abs(collider[i].mMin.x), std::abs(collider[i].mMax.x));
                    const float halfHeight = std::max(std::abs(collider[i].mMin.y), std::abs(collider[i].mMax.y));

                    if (halfWidth <= 0.0f || halfHeight <= 0.0f)
                        break;

                    vec2D minPt{ offsetPos.x - halfWidth, offsetPos.y - halfHeight };
                    vec2D maxPt{ offsetPos.x + halfWidth, offsetPos.y + halfHeight };
                    if (mousePos.x >= minPt.x && mousePos.x <= maxPt.x &&
                        mousePos.y >= minPt.y && mousePos.y <= maxPt.y)
                        return true;
                    break;
                }
                default:
                    break;
                }
            }
            return false;
        }

        // Fallback: use transform AABB if key has no collider.
        vec2D minPt = transform.GetMinPos();
        vec2D maxPt = transform.GetMaxPos();
        return mousePos.x > minPt.x && mousePos.x < maxPt.x &&
               mousePos.y > minPt.y && mousePos.y < maxPt.y;
    }

    bool EncryptionKey::IsKeyInteractableForCurrentLevel() const
    {
        const unsigned int level = p_GameManager.CurrentLevel();
        // Level 1: only Flamingo scene key is clickable.
        if (level <= 1)
            return mSceneKeyId == "scene:flamingobtn";

        // Level 2+: all animal scene keys are clickable.
        return true;
    }

    std::string EncryptionKey::BuildSceneKey() const
    {
        // Prefer scene identity from the currently lit camera button.
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Sprite>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();
            std::string texture = p_ecs.GetComponent<Sprite>(e).GetTexture();
            for (char& ch : tag)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            for (char& ch : texture)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (tag.size() >= 3 && tag.substr(tag.size() - 3) == "btn" &&
                texture.find("camerabuttonlit") != std::string::npos)
            {
                return std::string("scene:") + tag;
            }
        }

        // Fallback if expected button setup is missing.
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();
            for (char& ch : tag)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (tag == "flamingobtn" || tag == "horsebtn" || tag == "owlbtn" || tag == "sealbtn")
                return std::string("scene:") + tag;
        }

        // Final fallback keeps behavior deterministic.
        return std::string("entity:") + std::to_string(entityID);
    }

    void EncryptionKey::EnsureSceneAssignments()
    {
        if (!sSceneAssignments.empty())
            return;

        std::array<std::string, kEncryptionTextureKeys.size()> shuffledTextures{};
        for (size_t i = 0; i < kEncryptionTextureKeys.size(); ++i)
            shuffledTextures[i] = kEncryptionTextureKeys[i];

        std::mt19937 rng{ std::random_device{}() };
        std::shuffle(shuffledTextures.begin(), shuffledTextures.end(), rng);

        for (size_t i = 0; i < kSceneIds.size(); ++i)
            sSceneAssignments.emplace(kSceneIds[i], shuffledTextures[i]);
    }

    std::string EncryptionKey::ResolveAssignedTextureKey() const
    {
        auto it = sSceneAssignments.find(mSceneKeyId);
        if (it != sSceneAssignments.end())
            return it->second;

        return "Play/encryption_h";
    }

    void EncryptionKey::ResetRunState()
    {
        sCollectedKeys.clear();
        sSceneAssignments.clear();
    }

    // Shared collected-list lookup.
    bool EncryptionKey::IsCollected(const std::string& keyId) const
    {
        return sCollectedKeys.find(keyId) != sCollectedKeys.end();
    }

    void EncryptionKey::CollectKey()
    {
        if (mCollected)
            return;

        if (!IsCollected(mCollectionKeyId) && sCollectedKeys.size() < kMaxKeys)
        {
            sCollectedKeys.insert(mCollectionKeyId);
        }

        mCollected = true;
        mRevealActive = true;
        mRevealTimer = mRevealDuration;

        if (p_ecs.HaveComponent<Transform>(entityID))
        {
            auto& keyTransform = p_ecs.GetComponent<Transform>(entityID);
            keyTransform.SetPos(mHiddenPos.x, mHiddenPos.y);
            keyTransform.SetScale(mOriginalScale.x, mOriginalScale.y);
        }

        if (p_ecs.HaveComponent<Sprite>(entityID))
        {
            p_ecs.GetComponent<Sprite>(entityID).SetLayerOrder(mOriginalLayer);
        }
    }
}
