#include "pch.h"
#include "Scripts/ToyHideAndRevealOnClick.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Tag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "GUI/GUI.h"
#include "Math/Vmath.h"
#include "Scripts/GlobalGameManager.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace NocturnalEngine
{
    namespace
    {
        // Per-toy runtime memory:
        // - enteredOnce: whether this toy has already been seen in this run
        // - pendingHiddenPos: where to hide the toy the next time scene loads
        struct ToyPersistState
        {
            bool enteredOnce{ false };
            bool hasPendingHiddenPos{ false };
            vec2D pendingHiddenPos{};
        };

        std::unordered_map<std::string, ToyPersistState> gToyState{};
    }

    ToyHideAndRevealOnClick::ToyHideAndRevealOnClick()
        : mInput{ InputSystem::GetInstance().get() }
    {
    }

    void ToyHideAndRevealOnClick::Start()
    {
        // This script runs on a toy entity.
        // If no transform exists, we cannot place/show/hide the toy.
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        // Save original visual size so we can restore it after reveal.
        auto& toyT = p_ecs.GetComponent<Transform>(entityID);
        mOriginalScale = toyT.GetScale();

        // Save original collider so hidden toys can disable interaction
        // and later restore exact collider settings on reveal.
        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            mHadCollider = true;
            mOriginalCollider = p_ecs.GetComponent<Collider>(entityID);
        }

        // Find the environment bounds used for random hide positions.
        FindBG();
        if (!mHasBG)
            return;

        // Build stable key (usually from toy name) so this toy keeps state
        // across scene reloads during this game run.
        mToyKey = BuildToyKey();
        ToyPersistState& state = gToyState[mToyKey];

        // First time player enters this scene for this toy:
        // keep toy visible.
        if (!state.enteredOnce)
        {
            state.enteredOnce = true;
            ApplyVisibleState();
            return;
        }

        // Re-entering scene:
        // start hidden at the previously prepared random position.
        if (state.hasPendingHiddenPos)
            mHiddenPos = state.pendingHiddenPos;
        else
            mHiddenPos = RandomPointInsideBG();

        ApplyHiddenState();
    }

    void ToyHideAndRevealOnClick::Update(float dt)
    {
        UNREFERENCED_PARAMETER(dt);

        // Reveal logic only runs while toy is hidden.
        if (!mHasBG || !mHidden)
            return;

        // Reveal only when left mouse is clicked inside toy hit area.
        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && IsMouseNearToy())
        {
            // Keep toy at hidden location; just make it visible + re-enable collider.
            auto& toyT = p_ecs.GetComponent<Transform>(entityID);
            toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
            ApplyVisibleState();
        }
    }

    void ToyHideAndRevealOnClick::End()
    {
        // If gameplay is ending, clear all runtime toy memory for a fresh new run.
        if (p_GameManager.GetGameState() == GameManager::GameState::ENDING)
        {
            gToyState.clear();
            return;
        }

        if (!mHasBG)
            return;

        if (mToyKey.empty())
            mToyKey = BuildToyKey();

        // When leaving this scene, prepare where toy should be hidden next entry.
        ToyPersistState& state = gToyState[mToyKey];
        state.enteredOnce = true;
        state.pendingHiddenPos = RandomPointInsideBG();
        state.hasPendingHiddenPos = true;
    }

    void ToyHideAndRevealOnClick::FindBG()
    {
        mBG = 0;
        mHasBG = false;

        // Find entity with name containing "environment".
        // Its transform/collider defines valid random hide area.
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e) || !p_ecs.HaveComponent<Collider>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();
            for (char& ch : tag)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (tag.find("environment") != std::string::npos)
            {
                mBG = e;
                mHasBG = true;
                return;
            }
        }
    }

    vec2D ToyHideAndRevealOnClick::RandomPointInsideBG()
    {
        // Choose a random point inside environment bounds,
        // while keeping full toy size inside those bounds.
        auto& bgT = p_ecs.GetComponent<Transform>(mBG);
        auto& bgCol = p_ecs.GetComponent<Collider>(mBG)[0];
        const vec2D toyHalf = GetToyHalfExtents();

        // Fallback bounds from transform.
        vec2D minP = bgT.GetMinPos();
        vec2D maxP = bgT.GetMaxPos();

        // Prefer collider-based bounds when available.
        if (bgCol.mCol == Collider::ColliderType::rect || bgCol.mCol == Collider::ColliderType::box)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            minP = { center.x - bgCol.mMin.x, center.y - bgCol.mMin.y };
            maxP = { center.x + bgCol.mMax.x, center.y + bgCol.mMax.y };
        }
        else if (bgCol.mCol == Collider::ColliderType::circle ||
                 bgCol.mCol == Collider::ColliderType::bubble ||
                 bgCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            minP = { center.x - bgCol.mRadius, center.y - bgCol.mRadius };
            maxP = { center.x + bgCol.mRadius, center.y + bgCol.mRadius };
        }

        // Shrink by toy half-size so full toy stays inside bounds.
        minP.x += toyHalf.x;
        minP.y += toyHalf.y;
        maxP.x -= toyHalf.x;
        maxP.y -= toyHalf.y;

        if (maxP.x <= minP.x)
        {
            const float mid = (minP.x + maxP.x) * 0.5f;
            minP.x = maxP.x = mid;
        }
        if (maxP.y <= minP.y)
        {
            const float mid = (minP.y + maxP.y) * 0.5f;
            minP.y = maxP.y = mid;
        }

        std::uniform_real_distribution<float> distX(minP.x, maxP.x);
        std::uniform_real_distribution<float> distY(minP.y, maxP.y);
        vec2D candidate{ distX(mRng), distY(mRng) };

        // For circular environment collider, reject candidates outside allowed circle.
        if (bgCol.mCol == Collider::ColliderType::circle ||
            bgCol.mCol == Collider::ColliderType::bubble ||
            bgCol.mCol == Collider::ColliderType::bossball)
        {
            const vec2D center{ bgT.GetPos().x + bgCol.mOffset.x, bgT.GetPos().y + bgCol.mOffset.y };
            const float toyRadius = std::max(toyHalf.x, toyHalf.y);
            const float allowedRadius = std::max(0.0f, bgCol.mRadius - toyRadius);

            for (int i = 0; i < 12; ++i)
            {
                if (distance(candidate, center) <= allowedRadius)
                    return candidate;
                candidate = { distX(mRng), distY(mRng) };
            }
            return center;
        }

        return candidate;
    }

    vec2D ToyHideAndRevealOnClick::GetToyHalfExtents()
    {
        // Get approximate toy half-size.
        // Collider size is preferred so random positions are more accurate.
        if (mHadCollider)
        {
            const auto& c = mOriginalCollider[0];

            if (c.mCol == Collider::ColliderType::circle ||
                c.mCol == Collider::ColliderType::bubble ||
                c.mCol == Collider::ColliderType::bossball)
            {
                const float r = std::max(c.mRadius, 0.001f);
                return { r, r };
            }

            if (c.mCol == Collider::ColliderType::rect || c.mCol == Collider::ColliderType::box)
            {
                const float hx = std::max(std::max(std::abs(c.mMin.x), std::abs(c.mMax.x)), 0.001f);
                const float hy = std::max(std::max(std::abs(c.mMin.y), std::abs(c.mMax.y)), 0.001f);
                return { hx, hy };
            }
        }

        // If no collider, use transform scale as size approximation.
        const float hx = std::max(std::abs(mOriginalScale.x) * 0.5f, 0.001f);
        const float hy = std::max(std::abs(mOriginalScale.y) * 0.5f, 0.001f);
        return { hx, hy };
    }

    bool ToyHideAndRevealOnClick::IsMouseNearToy()
    {
        // Mouse position in world/UI space used by this project.
        const vec2D mouse{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };

        // If toy has collider data, test click against original collider
        // shifted to current hidden position.
        if (mHadCollider)
        {
            for (int i = 0; i < 2; ++i)
            {
                const Collider::Colpiece& c = mOriginalCollider[i];
                const vec2D colliderCenter{ mHiddenPos.x + c.mOffset.x, mHiddenPos.y + c.mOffset.y };

                switch (c.mCol)
                {
                case Collider::ColliderType::circle:
                case Collider::ColliderType::bubble:
                case Collider::ColliderType::bossball:
                    if (distance(mouse, colliderCenter) < c.mRadius)
                        return true;
                    break;

                case Collider::ColliderType::rect:
                case Collider::ColliderType::box:
                {
                    const vec2D minPt{ colliderCenter.x - c.mMin.x, colliderCenter.y - c.mMin.y };
                    const vec2D maxPt{ colliderCenter.x + c.mMax.x, colliderCenter.y + c.mMax.y };
                    if (mouse.x > minPt.x && mouse.x < maxPt.x &&
                        mouse.y > minPt.y && mouse.y < maxPt.y)
                    {
                        return true;
                    }
                    break;
                }

                default:
                    break;
                }
            }
            return false;
        }

        // Fallback hit-test: axis-aligned box from original toy scale.
        const vec2D half{ std::abs(mOriginalScale.x) * 0.5f, std::abs(mOriginalScale.y) * 0.5f };
        return (mouse.x > (mHiddenPos.x - half.x) && mouse.x < (mHiddenPos.x + half.x) &&
                mouse.y > (mHiddenPos.y - half.y) && mouse.y < (mHiddenPos.y + half.y));
    }

    std::string ToyHideAndRevealOnClick::BuildToyKey() const
    {
        // Prefer name-based key so same toy keeps same runtime state.
        if (p_ecs.HaveComponent<NameTag>(entityID))
        {
            std::string key = p_ecs.GetComponent<NameTag>(entityID).GetNameTag();
            for (char& ch : key)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            return key;
        }

        // Fallback when NameTag is missing.
        return "toy_" + std::to_string(static_cast<unsigned int>(entityID));
    }

    void ToyHideAndRevealOnClick::ApplyHiddenState()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        // Move to hidden spot and make sprite invisible.
        auto& toyT = p_ecs.GetComponent<Transform>(entityID);
        toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
        toyT.SetScale(0.0f, 0.0f);

        // Disable collider while hidden so it cannot be interacted with
        // through normal drag/collision systems.
        if (mHadCollider && p_ecs.HaveComponent<Collider>(entityID))
        {
            auto& c = p_ecs.GetComponent<Collider>(entityID);
            for (int i = 0; i < 2; ++i)
            {
                c[i].mRadius = 0.0f;
                c[i].mMin = { 0.0f, 0.0f };
                c[i].mMax = { 0.0f, 0.0f };
            }
        }

        mHidden = true;
    }

    void ToyHideAndRevealOnClick::ApplyVisibleState()
    {
        if (!p_ecs.HaveComponent<Transform>(entityID))
            return;

        // Restore original visual size.
        auto& toyT = p_ecs.GetComponent<Transform>(entityID);
        toyT.SetScale(mOriginalScale.x, mOriginalScale.y);

        // Restore original collider behavior.
        if (mHadCollider && p_ecs.HaveComponent<Collider>(entityID))
            p_ecs.GetComponent<Collider>(entityID) = mOriginalCollider;

        mHidden = false;
    }
}
