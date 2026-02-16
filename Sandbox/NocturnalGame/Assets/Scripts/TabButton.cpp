
/*!------------------------------------------------------------------------------------------------
	File: TabButton.cpp
	Project: NocturnalStudio

	Brief:
		 This file contains the Tab Button class definitions for tabbing through
		 How To Play manual.

	Primary Author:
		 Chang Qi Xuan Celest (100%)
		- Created the function definitions for button tabbing for How To Play scene.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
//#include "pch.h"
//#include "Scripts/TabButton.h"
//#include "ECS/ECS.h"
//#include "ECS/Components/Components.h"
//#include "Input/Input.h"
//
//namespace NocturnalEngine
//{
//    static Entity FindByName(const std::string& name)
//    {
//        for (Entity e = 0; e < p_ecs.GetTotalEntities(); ++e)
//        {
//            if (p_ecs.HaveComponent<NameTag>(e) &&
//                p_ecs.GetComponent<NameTag>(e).GetNameTag() == name)
//                return e;
//        }
//        return -1;
//    }
//
//    void TabButton::Start()
//    {
//        manualEntity = FindByName("Manual");
//
//        tabEntities[HTP_Tab::Overview] = FindByName("OverviewTab");
//        tabEntities[HTP_Tab::Flamingo] = FindByName("FlamingoTab");
//        tabEntities[HTP_Tab::Seal] = FindByName("SealTab");
//        tabEntities[HTP_Tab::Horse] = FindByName("HorseTab");
//        tabEntities[HTP_Tab::Owl] = FindByName("OwlTab");
//        tabEntities[HTP_Tab::Health] = FindByName("HealthTab");
//        tabEntities[HTP_Tab::Hunger] = FindByName("HungerTab");
//        tabEntities[HTP_Tab::Sanity] = FindByName("SanityTab");
//
//        for (auto& [_, e] : tabEntities)
//        {
//            if (p_ecs.HaveComponent<Transform>(e))
//            {
//                baseX[e] = p_ecs.GetComponent<Transform>(e).GetPos().x;
//            }
//        }
//
//        SelectTab(HTP_Tab::Overview);
//    }
//
//    void TabButton::Update(float)
//    {
//        //no per-frame logic needed
//    }
//
//    void TabButton::SelectTab(HTP_Tab tab)
//    {
//        if (manualEntity == -1)
//        {
//            std::cout << "Manual entity not found\n";
//            return;
//        }
//        currentTab = tab;
//
//        auto& manualSprite = p_ecs.GetComponent<Sprite>(manualEntity);
//
//        switch (tab)
//        {
//        case HTP_Tab::Overview: manualSprite.SetTexture("overviewhtp"); break;
//        case HTP_Tab::Flamingo: manualSprite.SetTexture("flamingohtp"); break;
//        case HTP_Tab::Seal:     manualSprite.SetTexture("sealhtp"); break;
//        case HTP_Tab::Horse:    manualSprite.SetTexture("horsehtp"); break;
//        case HTP_Tab::Owl:      manualSprite.SetTexture("owlhtp"); break;
//        case HTP_Tab::Health:   manualSprite.SetTexture("healthhtp"); break;
//        case HTP_Tab::Hunger:   manualSprite.SetTexture("hungerhtp"); break;
//        case HTP_Tab::Sanity:   manualSprite.SetTexture("sanityhtp"); break;
//        }
//
//        for (auto& [t, e] : tabEntities)
//        {
//            auto& tr = p_ecs.GetComponent<Transform>(e);
//            auto pos = tr.GetPos();
//            pos.x = baseX[e] + ((t == tab) ? selectedOffsetX : 0.0f);
//            tr.SetPos(pos);
//        }
//    }
//}
