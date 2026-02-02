
#include "pch.h"
#include "Scripts/ToyHideAndRevealOnClick.h"
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/NameTag.h"
#include "ECS/Components/Tag.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Collider.h"
#include "GUI/GUI.h"
#include "Math/Vmath.h"
#include <ctime>
#include <cstdlib>


namespace NocturnalEngine
{
    ToyHideAndRevealOnClick::ToyHideAndRevealOnClick()
        : mInput{ InputSystem::GetInstance().get() }
    {
    }


    void ToyHideAndRevealOnClick::Start()
    {
        // --- AFTER THIS LINE: cache toy original scale ---
        // (this script is attached to the Toy entity, so entityID is the toy)
        if (!p_ecs.HaveComponent<Transform>(entityID)) return;

        auto& toyT = p_ecs.GetComponent<Transform>(entityID);
        mOriginalScale = toyT.GetScale();

        // --- AFTER THIS LINE: cache collider & disable later ---
        if (p_ecs.HaveComponent<Collider>(entityID))
        {
            mHadCollider = true;
            mOriginalCollider = p_ecs.GetComponent<Collider>(entityID);
        }

        // --- AFTER THIS LINE: find BG & pick hidden spot ---
        FindBG();
        if (!mHasBG) return;

        mHiddenPos = RandomPointInsideBG();

        static bool sSeeded = false;
        if (!sSeeded)
        {
            sSeeded = true;
            // seed with time so every run is different
            srand(static_cast<unsigned>(time(nullptr)));
        }


        // --- AFTER THIS LINE: hide toy (scale 0) and move it to hidden pos ---
        toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
        toyT.SetScale(0.0f, 0.0f);

        // --- AFTER THIS LINE: disable collider so invisible toy can’t be dragged/clicked ---
        if (mHadCollider)
        {
            auto& c = p_ecs.GetComponent<Collider>(entityID);
            // zero out both slots
            for (int i = 0; i < 2; ++i)
            {
                c[i].mRadius = 0.0f;
                c[i].mMin = { 0.0f, 0.0f };
                c[i].mMax = { 0.0f, 0.0f };
            }
        }

        mHidden = true;
    }

    void ToyHideAndRevealOnClick::Update(float dt)
    {
        UNREFERENCED_PARAMETER(dt);
        if (!mHasBG) return;
        if (!mHidden) return;

        // --- AFTER THIS LINE: click to reveal ---
        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (IsMouseNearHiddenSpot())
            {
                // reveal toy
                auto& toyT = p_ecs.GetComponent<Transform>(entityID);
                toyT.SetPos(mHiddenPos.x, mHiddenPos.y);
                toyT.SetScale(mOriginalScale.x, mOriginalScale.y);

                // restore collider
                if (mHadCollider)
                {
                    p_ecs.GetComponent<Collider>(entityID) = mOriginalCollider;
                }

                mHidden = false; // stays visible forever after first find
            }
        }
    }

    void ToyHideAndRevealOnClick::End()
    {
        // optional: restore on scene unload
        if (p_ecs.HaveComponent<Transform>(entityID))
        {
            auto& toyT = p_ecs.GetComponent<Transform>(entityID);
            toyT.SetScale(mOriginalScale.x, mOriginalScale.y);
        }
        if (mHadCollider && p_ecs.HaveComponent<Collider>(entityID))
        {
            p_ecs.GetComponent<Collider>(entityID) = mOriginalCollider;
        }
    }

    void ToyHideAndRevealOnClick::FindBG()
    {
        mBG = 0;
        mHasBG = false;

        // change "BG" to whatever NameTag you use for your background entity
        for (Entity e : p_ecs.GetAliveEntitiesSet())
        {
            if (!p_ecs.HaveComponent<NameTag>(e) || !p_ecs.HaveComponent<Transform>(e))
                continue;

            std::string tag = p_ecs.GetComponent<NameTag>(e).GetNameTag();

            // make lowercase
            for (char& ch : tag) ch = (char)tolower((unsigned char)ch);

            // match anything that contains "bg"
            if (tag.find("bg") != std::string::npos)
            {
                mBG = e;
                mHasBG = true;
                return;
            }

        }
    }

    vec2D ToyHideAndRevealOnClick::RandomPointInsideBG() const
    {
        auto& bgT = p_ecs.GetComponent<Transform>(mBG);

        // assuming BG is a rectangle in screen space
        vec2D minP = bgT.GetMinPos();
        vec2D maxP = bgT.GetMaxPos();

        // keep it away from edges a bit
        float pad = 40.0f;
        minP.x += pad; minP.y += pad;
        maxP.x -= pad; maxP.y -= pad;

        // if padding makes the range invalid, remove padding
        if (maxP.x <= minP.x) { minP.x -= pad; maxP.x += pad; }
        if (maxP.y <= minP.y) { minP.y -= pad; maxP.y += pad; }


        float rx = minP.x + (float)(rand()) / (float)RAND_MAX * (maxP.x - minP.x);
        float ry = minP.y + (float)(rand()) / (float)RAND_MAX * (maxP.y - minP.y);

        return { rx, ry };
    }

    bool ToyHideAndRevealOnClick::IsMouseNearHiddenSpot() const
    {
        vec2D mouse{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        return distance(mouse, mHiddenPos) <= mRevealRadius;
    }
}
