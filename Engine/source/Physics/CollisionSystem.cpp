/*!------------------------------------------------------------------------------------------------
    File: CollisionSystem.cpp
    Project: NocturnalStudio

	Brief:
		This file contains the implementation of the CollisionSystem class which is responsible 
        for handling collision detection and response within the physics engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the CollisionSystem class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Physics/CollisionSystem.h"
#include "ResourceManager/ResourceManager.h"
#include "ECS/Components/Components.h"
#include "Math/Physics.h"
#include <graphics/Graphics.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <limits>

namespace NocturnalEngine {
    /*!*************************************************************************
    This function initialises the system. As there are no data members that require
    initialization, this function is empty
    ****************************************************************************/
    void CollisionSystem::Init() {

    }
    /*!*************************************************************************
    This function runs the logic of the system. It checks the list of entities with
    a collision component in pairs to see which of them are colliding. If two objects
    are colliding, collision response is dictated by the type of collider the object
    has
    ****************************************************************************/
    void CollisionSystem::Update(float dt) {
        (void)dt;
        Timer::GetInstance().Start(Systems::COLLISION);
        Timer::GetInstance().GetDT(Systems::COLLISION);

        auto IsLayerDisabled = [&](Entity e) -> bool
            {
                if (!p_ecs.HaveComponent<Sprite>(e)) return false; // no sprite = not layer-controlled
                int layer = p_ecs.GetComponent<Sprite>(e).LayerOrder;

                if (layer < 0) layer = 0;
                if (layer >= NocturnalEngine::Graphic::MAX_LAYERS)
                    layer = NocturnalEngine::Graphic::MAX_LAYERS - 1;

                return !NocturnalEngine::Graphic::layerEnabled[layer];
            };


        //iterate through array of entities
        for (auto const& i : mEntities) {
            auto& col1 = p_ecs.GetComponent<Collider>(i);
            for (int a = 0; a < 2; a++) {
                col1[a].mHit = 0;
                col1[a].mCollisionNormal = vec2D();
            }
        }

        struct BroadEntry {
            Entity entity;
            vec2D min;
            vec2D max;
        };

        const float cellSize = 256.0f;
        auto HashCell = [](int x, int y) -> std::uint64_t {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32) |
                static_cast<std::uint32_t>(y);
        };

        auto UpdateBounds = [](vec2D& minOut, vec2D& maxOut, const vec2D& newMin, const vec2D& newMax)
        {
            if (newMin.x < minOut.x) minOut.x = newMin.x;
            if (newMin.y < minOut.y) minOut.y = newMin.y;
            if (newMax.x > maxOut.x) maxOut.x = newMax.x;
            if (newMax.y > maxOut.y) maxOut.y = newMax.y;
        };

        std::vector<BroadEntry> broadEntries;
        broadEntries.reserve(mEntities.size());

        for (auto const& entity : mEntities) {
            if (IsLayerDisabled(entity)) continue;

            if (!p_ecs.HaveComponent<Collider>(entity) || !p_ecs.HaveComponent<RigidBody>(entity) || !p_ecs.HaveComponent<Transform>(entity))
                continue;

            auto& col = p_ecs.GetComponent<Collider>(entity);
            auto& rigid = p_ecs.GetComponent<RigidBody>(entity);
            auto& trans = p_ecs.GetComponent<Transform>(entity);

            vec2D boundsMin{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
            vec2D boundsMax{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

            for (int a = 0; a < 2; ++a) {
                auto& piece = col[a];
                if (!piece.is_Alive || piece.mCol == Collider::ColliderType::none)
                    continue;

                vec2D basePos = rigid.GetNextPos();
                if (basePos.x == 0.0f && basePos.y == 0.0f) {
                    basePos = trans.GetPos();
                }
                vec2D offset = basePos + piece.mOffset;

                switch (piece.mCol) {
                case Collider::ColliderType::circle:
                case Collider::ColliderType::bubble:
                case Collider::ColliderType::bossball:
                case Collider::ColliderType::dashCirc:
                    UpdateBounds(boundsMin, boundsMax, offset - vec2D(piece.mRadius, piece.mRadius), offset + vec2D(piece.mRadius, piece.mRadius));
                    break;
                case Collider::ColliderType::rect:
                case Collider::ColliderType::box:
                    UpdateBounds(boundsMin, boundsMax, offset - piece.mMin, offset + piece.mMax);
                    break;
                default:
                    break;
                }
            }

            if (boundsMin.x <= boundsMax.x && boundsMin.y <= boundsMax.y) {
                broadEntries.push_back({ entity, boundsMin, boundsMax });
            }
        }

        std::unordered_map<std::uint64_t, std::vector<std::size_t>> grid;
        grid.reserve(broadEntries.size());

        for (std::size_t idx = 0; idx < broadEntries.size(); ++idx) {
            const auto& entry = broadEntries[idx];

            int startX = static_cast<int>(std::floor(entry.min.x / cellSize));
            int startY = static_cast<int>(std::floor(entry.min.y / cellSize));
            int endX = static_cast<int>(std::floor(entry.max.x / cellSize));
            int endY = static_cast<int>(std::floor(entry.max.y / cellSize));

            for (int x = startX; x <= endX; ++x) {
                for (int y = startY; y <= endY; ++y) {
                    grid[HashCell(x, y)].push_back(idx);
                }
            }
        }

        std::unordered_set<std::uint64_t> uniquePairs;
        std::vector<std::pair<Entity, Entity>> candidatePairs;
        candidatePairs.reserve(broadEntries.size() * 2);

        for (auto& cell : grid) {
            auto& bucket = cell.second;
            for (std::size_t a = 0; a < bucket.size(); ++a) {
                for (std::size_t b = a + 1; b < bucket.size(); ++b) {
                    Entity e1 = broadEntries[bucket[a]].entity;
                    Entity e2 = broadEntries[bucket[b]].entity;
                    Entity low = std::min(e1, e2);
                    Entity high = std::max(e1, e2);
                    std::uint64_t key = (static_cast<std::uint64_t>(low) << 32) | high;
                    if (uniquePairs.insert(key).second) {
                        candidatePairs.emplace_back(e1, e2);
                    }
                }
            }
        }

        for (int f = 0; f < 5; f++) {
            for (auto const& pair : candidatePairs) {
                auto i = pair.first;
                auto j = pair.second;
                // Layering system: if either entity is on a disabled layer, skip collision checks
                if (IsLayerDisabled(i) || IsLayerDisabled(j))
                {
                    continue;
                }
                auto& col1 = p_ecs.GetComponent<Collider>(i);
                auto& col2 = p_ecs.GetComponent<Collider>(j);
                auto& rigid1 = p_ecs.GetComponent<RigidBody>(i);
                auto& rigid2 = p_ecs.GetComponent<RigidBody>(j);
                auto& trans1 = p_ecs.GetComponent<Transform>(i);
                auto& trans2 = p_ecs.GetComponent<Transform>(j);
                //broad phase sweep
                if (distance(rigid1.GetNextPos(), rigid2.GetNextPos()) < 100) {
                    for (int a = 0; a < 2; a++) {
                        for (int b = 0; b < 2; b++) {
                                    Collider::ColliderType e1 = col1[a].mCol;
                                    Collider::ColliderType e2 = col2[b].mCol;
                                    vec2D offset1 = rigid1.GetNextPos() + col1[a].mOffset;
                                    vec2D offset2 = rigid2.GetNextPos() + col2[b].mOffset;
                                    if (col1[a].is_Alive && col2[b].is_Alive) {
                                        if (e1 == Collider::ColliderType::circle) {
                                            if (e2 == Collider::ColliderType::box) {
                                                rigid2.SetNextPos(trans2.GetPos());
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                vec2D max2 = offset2 + col2[b].mMax;
                                                vec2D min2 = offset2 - col2[b].mMin;
                                                if (ecm.simpleCircleRect(offset1, col1[a].mRadius, max2, min2, offset2)) {
                                                    vec2D norm1;
                                                    if (((offset1.y - col1[a].mRadius) < (max2.y) && (offset1.y + col1[a].mRadius) >(min2.y)) && (offset1.x - col1[a].mRadius) <= min2.x) {
                                                        norm1.x = -1;
                                                    }
                                                    else if (((offset1.y - col1[a].mRadius) < (max2.y) && (offset1.y + col1[a].mRadius) >(min2.y)) && (offset1.x + col1[a].mRadius) >= max2.x) {
                                                        norm1.x = 1;
                                                    }
                                                    if (((offset1.x - col1[a].mRadius) < (max2.x) && (offset1.x + col1[a].mRadius) >(min2.x)) && (offset1.y + col1[a].mRadius) >= max2.y) {
                                                        norm1.y = 1;
                                                    }
                                                    else if (((offset1.x - col1[a].mRadius) < (max2.x) && (offset1.x + col1[a].mRadius) >(min2.x)) && (offset1.y - col1[a].mRadius) <= min2.y) {
                                                        norm1.y = -1;
                                                    }
                                                    Normalize(norm1, norm1);
                                                    col1[a].mHit = 1;
                                                    col1[a].mCollisionNormal = norm1;

                                                    vec2D response = rigid1.GetVel();
                                                    vec2D normal = norm1;
                                                    float dotProd = dotProduct(normal, response);
                                                    if (dotProd <= 0) {
                                                        normal = normal * dotProd;
                                                        response -= normal;
                                                        rigid1.SetVel(response);
                                                        vec2D nextPos = trans1.GetPos() + rigid1.GetVel();
                                                        rigid1.SetNextPos(nextPos);
                                                    }
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::circle) {
                                                if (ecm.simpleCircleCircle(offset1, offset2, col1[a].mRadius, col2[b].mRadius)) {
                                                    col1[a].mHit = 1;
                                                    vec2D norm1 = offset1 - offset2;
                                                    Normalize(norm1, norm1);
                                                    col1[a].mCollisionNormal = norm1;

                                                    vec2D response1 = rigid1.GetVel();
                                                    vec2D normal1 = norm1;
                                                    float dotProd1 = dotProduct(normal1, response1);
                                                    if (dotProd1 <= 0) {
                                                        normal1 = normal1 * dotProd1;
                                                        response1 -= normal1;
                                                        if (p_ecs.GetComponent<NameTag>(j).GetNameTag() == "player") {
                                                            rigid1.SetVel(vec2D(0, 0));
                                                        }
                                                        else {
                                                            rigid1.SetVel(response1);
                                                        }
                                                        vec2D nextPos1 = trans1.GetPos() + rigid1.GetVel();
                                                        rigid1.SetNextPos(nextPos1);
                                                    }

                                                    col2[b].mHit = 1;
                                                    vec2D norm2 = -norm1;
                                                    col2[b].mCollisionNormal = norm2;

                                                    vec2D response2 = rigid2.GetVel();
                                                    vec2D normal2 = norm2;
                                                    float dotProd2 = dotProduct(normal2, response2);
                                                    if (dotProd2 <= 0) {
                                                        normal2 = normal2 * dotProd2;
                                                        response2 -= normal2;
                                                        if (p_ecs.GetComponent<NameTag>(i).GetNameTag() == "player") {
                                                            rigid2.SetVel(vec2D(0, 0));
                                                        }
                                                        else {
                                                            rigid2.SetVel(response2);
                                                        }
                                                        vec2D nextPos2 = trans2.GetPos() + rigid2.GetVel();
                                                        rigid2.SetNextPos(nextPos2);
                                                    }
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::dashCirc) {
                                                if (ecm.simpleCircleCircle(offset1, offset2, col1[a].mRadius, col2[b].mRadius)) {
                                                    vec2D nextPos1 = trans1.GetPos() - (rigid1.GetVel() * 3);
                                                    rigid1.SetNextPos(nextPos1);
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::bubble) {
                                                rigid2.SetNextPos(trans2.GetPos());
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                if (ecm.simpleCircleCircle(offset1, offset2, col1[a].mRadius, col2[b].mRadius)) {
                                                    col2[b].mHit = 3;
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::rect) {
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                vec2D max2 = offset2 + col2[b].mMax;
                                                vec2D min2 = offset2 - col2[b].mMin;
                                                if (ecm.simpleCircleRect(offset1, col1[a].mRadius, max2, min2, offset2)) {
                                                    col1[a].mHit = 2;
                                                    col2[b].mHit = 4;
                                                }
                                            }
                                        }
                                        else if (e1 == Collider::ColliderType::rect) {
                                            if (e2 == Collider::ColliderType::circle) {
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                vec2D max1 = offset1 + col1[a].mMax;
                                                vec2D min1 = offset1 - col1[a].mMin;
                                                if (ecm.simpleCircleRect(offset2, col2[b].mRadius, max1, min1, offset1)) {
                                                    col2[b].mHit = 2;
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::bossball) {
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                vec2D max1 = offset1 + col1[a].mMax;
                                                vec2D min1 = offset1 - col1[a].mMin;
                                                if (ecm.simpleCircleRect(offset2, col2[b].mRadius, max1, min1, offset1)) {
                                                    col2[b].mHit = 2;
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::rect) {
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                vec2D max1 = offset1 + col1[a].mMax;
                                                vec2D min1 = offset1 - col1[a].mMin;
                                                vec2D max2 = offset2 + col2[b].mMax;
                                                vec2D min2 = offset2 - col2[b].mMin;
                                                if (ecm.simpleRectRect(max1, min1, max2, min2)) {
                                                }
                                            }
                                        }
                                        else if (e1 == Collider::ColliderType::box) {
                                            if (e2 == Collider::ColliderType::circle) {
                                                rigid1.SetNextPos(trans1.GetPos());
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                vec2D max1 = offset1 + col1[a].mMax;
                                                vec2D min1 = offset1 - col1[a].mMin;
                                                if (ecm.simpleCircleRect(offset2, col2[b].mRadius, max1, min1, offset1)) {
                                                    vec2D norm2;
                                                    if (((offset2.y - col2[b].mRadius) < (max1.y) && (offset2.y + col2[b].mRadius) >(min1.y)) && (offset2.x - col2[b].mRadius) <= min1.x) {
                                                        norm2.x = -1;
                                                    }
                                                    else if (((offset2.y - col2[b].mRadius) < (max1.y) && (offset2.y + col2[b].mRadius) >(min1.y)) && (offset2.x + col2[b].mRadius) >= max1.x) {
                                                        norm2.x = 1;
                                                    }
                                                    if (((offset2.x - col2[b].mRadius) < (max1.x) && (offset2.x + col2[b].mRadius) >(min1.x)) && (offset2.y + col2[b].mRadius) >= max1.y) {
                                                        norm2.y = 1;
                                                    }
                                                    else if (((offset2.x - col2[b].mRadius) < (max1.x) && (offset2.x + col2[b].mRadius) >(min1.x)) && (offset2.y - col2[b].mRadius) <= min1.y) {
                                                        norm2.y = -1;
                                                    }
                                                    Normalize(norm2, norm2);
                                                    col2[b].mHit = 1;
                                                    col2[b].mCollisionNormal = norm2;

                                                    vec2D response = rigid2.GetVel();
                                                    vec2D normal = norm2;
                                                    float dotProd = dotProduct(normal, response);
                                                    if (dotProd <= 0) {
                                                        normal = normal * dotProd;
                                                        response -= normal;
                                                        rigid2.SetVel(response);
                                                        vec2D nextPos = trans2.GetPos() + rigid2.GetVel();
                                                        rigid2.SetNextPos(nextPos);
                                                    }
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::dashCirc) {
                                                rigid1.SetNextPos(trans1.GetPos());
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                vec2D max1 = offset1 + col1[a].mMax;
                                                vec2D min1 = offset1 - col1[a].mMin;
                                                if (ecm.simpleCircleRect(offset2, col2[b].mRadius, max1, min1, offset1)) {
                                                    vec2D norm2;
                                                    if (((offset2.y - col2[b].mRadius) < (max1.y) && (offset2.y + col2[b].mRadius) > (min1.y)) && (offset2.x - col2[b].mRadius) <= min1.x) {
                                                        norm2.x = -1;
                                                    }
                                                    else if (((offset2.y - col2[b].mRadius) < (max1.y) && (offset2.y + col2[b].mRadius) > (min1.y)) && (offset2.x + col2[b].mRadius) >= max1.x) {
                                                        norm2.x = 1;
                                                    }
                                                    if (((offset2.x - col2[b].mRadius) < (max1.x) && (offset2.x + col2[b].mRadius) > (min1.x)) && (offset2.y + col2[b].mRadius) >= max1.y) {
                                                        norm2.y = 1;
                                                    }
                                                    else if (((offset2.x - col2[b].mRadius) < (max1.x) && (offset2.x + col2[b].mRadius) > (min1.x)) && (offset2.y - col2[b].mRadius) <= min1.y) {
                                                        norm2.y = -1;
                                                    }
                                                    Normalize(norm2, norm2);
                                                    col2[b].mHit = 1;
                                                    col2[b].mCollisionNormal = norm2;

                                                    vec2D response = rigid2.GetVel();
                                                    vec2D normal = norm2;
                                                    float dotProd = dotProduct(normal, response);
                                                    if (dotProd <= 0) {
                                                        normal = normal * dotProd;
                                                        response -= normal;
                                                        rigid2.SetVel(response);
                                                        vec2D nextPos = trans2.GetPos() + rigid2.GetVel();
                                                        rigid2.SetNextPos(nextPos);
                                                    }
                                                }
                                            }
                                        }
                                        else if (e1 == Collider::ColliderType::bubble) {
                                            if (e2 == Collider::ColliderType::circle) {
                                                rigid1.SetNextPos(trans1.GetPos());
                                                offset1 = trans1.GetPos() + col1[a].mOffset;
                                                if (ecm.simpleCircleCircle(offset1, offset2, col1[a].mRadius, col2[b].mRadius)) {
                                                    col1[a].mHit = 3;
                                                }
                                            }
                                        }
                                        else if (e1 == Collider::ColliderType::bossball) {
                                            if (e2 == Collider::ColliderType::rect) {
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                vec2D max2 = offset2 + col2[b].mMax;
                                                vec2D min2 = offset2 - col2[b].mMin;
                                                if (ecm.simpleCircleRect(offset1, col1[a].mRadius, max2, min2, offset2)) {
                                                    col1[a].mHit = 2;
                                                }
                                            }
                                        }
                                        else if (e1 == Collider::ColliderType::dashCirc) {
                                            if (e2 == Collider::ColliderType::box) {
                                                rigid2.SetNextPos(trans2.GetPos());
                                                offset2 = trans2.GetPos() + col2[b].mOffset;
                                                vec2D max2 = offset2 + col2[b].mMax;
                                                vec2D min2 = offset2 - col2[b].mMin;
                                                if (ecm.simpleCircleRect(offset1, col1[a].mRadius, max2, min2, offset2)) {
                                                    vec2D norm1;
                                                    if (((offset1.y - col1[a].mRadius) < (max2.y) && (offset1.y + col1[a].mRadius) > (min2.y)) && (offset1.x - col1[a].mRadius) <= min2.x) {
                                                        norm1.x = -1;
                                                    }
                                                    else if (((offset1.y - col1[a].mRadius) < (max2.y) && (offset1.y + col1[a].mRadius) > (min2.y)) && (offset1.x + col1[a].mRadius) >= max2.x) {
                                                        norm1.x = 1;
                                                    }
                                                    if (((offset1.x - col1[a].mRadius) < (max2.x) && (offset1.x + col1[a].mRadius) > (min2.x)) && (offset1.y + col1[a].mRadius) >= max2.y) {
                                                        norm1.y = 1;
                                                    }
                                                    else if (((offset1.x - col1[a].mRadius) < (max2.x) && (offset1.x + col1[a].mRadius) > (min2.x)) && (offset1.y - col1[a].mRadius) <= min2.y) {
                                                        norm1.y = -1;
                                                    }
                                                    Normalize(norm1, norm1);
                                                    col1[a].mHit = 1;
                                                    col1[a].mCollisionNormal = norm1;

                                                    vec2D response = rigid1.GetVel();
                                                    vec2D normal = norm1;
                                                    float dotProd = dotProduct(normal, response);
                                                    if (dotProd <= 0) {
                                                        normal = normal * dotProd;
                                                        response -= normal;
                                                        rigid1.SetVel(response);
                                                        vec2D nextPos = trans1.GetPos() + rigid1.GetVel();
                                                        rigid1.SetNextPos(nextPos);
                                                    }
                                                }
                                            }
                                            if (e2 == Collider::ColliderType::circle) {
                                                if (ecm.simpleCircleCircle(offset1, offset2, col1[a].mRadius, col2[b].mRadius)) {
                                                    vec2D nextPos2 = trans2.GetPos() - (rigid2.GetVel() * 3);
                                                    rigid2.SetNextPos(nextPos2);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
            }
        }
        
        Timer::GetInstance().Update(Systems::COLLISION);
    }
    /*!*************************************************************************
    This function ends the system. As there are no data members that require
    initialization, there are no data members that need to be un-initialised.
    Therefore this function is empty
    ****************************************************************************/
    void CollisionSystem::End() {}
}