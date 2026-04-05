#include "pch.h"
#include "Scripts/ToyHideAndRevealOnClick.h"
#include "ECS/Components/ItemsAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Tag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Sprite.h"
#include "GUI/GUI.h"
#include "Scripts/GlobalGameManager.h"
#include "Math/Random.h"

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
        checkPeriod = p_GameManager.GetCheckPeriod()*5.f;

        //find animal entity
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (p_ecs.HaveComponent<AnimalAttributes>(e))
            {
                animalEntity = e;
                continue;
            }
            if (p_ecs.HaveComponent<NameTag>(e) &&
                p_ecs.GetComponent<NameTag>(e).GetNameTag() == "Bush")
            {
                if (p_ecs.HaveComponent<Transform>(e) &&
                    p_ecs.HaveComponent<Sprite>(e) &&
                    p_ecs.HaveComponent<Collider>(e))
                {
                    hidingSpots.push_back(e);
                }
            }
        }

        if (animalEntity == MAX_ENTITIES) return;

        AnimalAttributes& aa{ p_ecs.GetComponent<AnimalAttributes>(animalEntity) };

        if (p_ecs.HaveComponent<ItemsAttributes>(entityID)) {
            ItemsAttributes& iA{ p_ecs.GetComponent<ItemsAttributes>(entityID) };

            GameManager::ToyState ts{ p_GameManager.LoadToyState(aa.WhatKindOfAnimalIsThis()) };

            iA.toyIsHidden = ts.isHiding;
            iA.hidingIndex = ts.hidingIndex;
        }
        if (p_ecs.HaveComponent<Transform>(entityID))ogPos = p_ecs.GetComponent<Transform>(entityID).GetPos();
        if (p_ecs.HaveComponent<Sprite>(entityID))ogLayer = p_ecs.GetComponent<Sprite>(entityID).LayerOrder;
       
    }

    void ToyHideAndRevealOnClick::Update(float dt)
    {
        if (!p_ecs.HaveComponent<ItemsAttributes>(entityID)) return;
        if (!p_ecs.HaveComponent<AnimalAttributes>(animalEntity)) return;
        if (!p_ecs.HaveComponent<Transform>(entityID)) return;

        ItemsAttributes& iA{ p_ecs.GetComponent<ItemsAttributes>(entityID) };
        AnimalAttributes& aa{ p_ecs.GetComponent<AnimalAttributes>(animalEntity) };
        aa.timeSinceSanity += dt;

        if(!iA.toyIsHidden) iA.toyIsHidden = CheckHide(aa);

        if (iA.toyIsHidden && !hasBeenHidden) iA.hidingIndex=HideToy();
            
        IsMouseNearHiddenSpot(iA, aa);


    }

    void ToyHideAndRevealOnClick::End()
    {
        if (!p_ecs.HaveComponent<ItemsAttributes>(entityID)) return;

        ItemsAttributes& iA{ p_ecs.GetComponent<ItemsAttributes>(entityID) };
        p_GameManager.SaveToyState(iA.toyIsHidden, iA.hidingIndex);
    }

    bool ToyHideAndRevealOnClick::CheckHide(AnimalAttributes& aa)
    {
        if (aa.sanityDecayRate <= 0.0)aa.sanityDecayRate = 0.00000001;

        if (aa.timeSinceSanity>=checkPeriod/aa.sanityDecayRate)
        {
            aa.timeSinceSanity = 0.f;

            if (Random::Range(0,1))
            {
                return true;

            }
        }
        return false;
    }

    int ToyHideAndRevealOnClick::HideToy()
    {
        int hideIndex{ 0 };

        Transform& itemTransform{ p_ecs.GetComponent<Transform>(entityID) };
        if (!hidingSpots.empty())
        {
            hideIndex = Random::Range(0, static_cast<int>(hidingSpots.size()) - 1);
            
            Sprite& hidingSprite{ p_ecs.GetComponent<Sprite>(hidingSpots[hideIndex]) }, &itemSprite{ p_ecs.GetComponent<Sprite>(entityID) };
            
            int hidingLayer = hidingSprite.LayerOrder > 1 ? hidingSprite.LayerOrder - 1: 1;
            itemSprite.SetLayerOrder(hidingLayer);
            itemTransform.SetPos(GetRandomPointInBush(hidingSpots[hideIndex]));

        }
        //std::cout << "toy hidden\n";
        hasBeenHidden = true;

        return hideIndex;
    }

    vec2D ToyHideAndRevealOnClick::GetRandomPointInBush(Entity bushEntity) const
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


    void ToyHideAndRevealOnClick::IsMouseNearHiddenSpot(ItemsAttributes& iA, AnimalAttributes& aa)
    {
        if (mInput->MousePressed(0) && p_ecs.HaveComponent<Transform>(entityID)) {
            vec2D mPos{ p_GUI->MousePosition.x,p_GUI->MousePosition.y };
            Transform& t = p_ecs.GetComponent<Transform>(entityID);

            if (p_ecs.HaveComponent<Collider>(entityID))
            {
                Collider& c = p_ecs.GetComponent<Collider>(entityID);
                for (int i = 0; i < 2; i++)
                {
                    vec2D offsetPos{ t.GetPos().x + c[i].mOffset.x ,t.GetPos().y + c[i].mOffset.y };
                    switch (c[i].mCol)
                    {
                    case Collider::ColliderType::circle:
                    case Collider::ColliderType::bubble:
                        if (distance(mPos, offsetPos) < c[i].mRadius) {
                            
                            ResetThing(iA, aa, t, p_ecs.GetComponent<Sprite>(entityID));
                        }
                        break;
                    case Collider::ColliderType::rect:
                    case Collider::ColliderType::box:
                        vec2D minPt{ offsetPos.x - c[i].mMin.x,offsetPos.y - c[i].mMin.y }, maxPt{ offsetPos.x + c[i].mMax.x,offsetPos.y + c[i].mMax.y };
                        if (mPos.x > minPt.x && mPos.x<maxPt.x && mPos.y>minPt.y && mPos.y < maxPt.y)
                        {
                            ResetThing(iA, aa, t, p_ecs.GetComponent<Sprite>(entityID));

                        }
                        break;
                    }
                }

            }
            else
            {


                if (mPos.x > t.GetMinPos().x && mPos.x < t.GetMaxPos().x &&
                    mPos.y>t.GetMinPos().y && mPos.y < t.GetMaxPos().y)
                {
                    ResetThing(iA, aa, t, p_ecs.GetComponent<Sprite>(entityID));


                }
            }
        }
    }

    void ToyHideAndRevealOnClick::ResetThing(ItemsAttributes& iA, AnimalAttributes& aa,Transform& t, Sprite& s){
        iA.toyIsHidden = false;
        hasBeenHidden = false;
        aa.timeSinceSanity = -10.f;

        t.SetPos(ogPos);
        s.SetLayerOrder(ogLayer);
    }
}
