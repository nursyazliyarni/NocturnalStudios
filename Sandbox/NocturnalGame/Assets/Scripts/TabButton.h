
/*!------------------------------------------------------------------------------------------------
	File: TabButton.h
	Project: NocturnalStudio

	Brief:
		 This file contains the Tab Button class declaration for tabbing through
		 How To Play manual.

	Primary Author:
		 Chang Qi Xuan Celest (100%)
		- Created the function declaration for button tabbing for How To Play scene.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

//#pragma once
//#include "Scripts/IScript.h"
//#include <unordered_map>
//
//namespace NocturnalEngine
//{
//    enum class HTP_Tab
//    {
//        Overview,
//        Flamingo,
//        Seal,
//        Horse,
//        Owl,
//        Health,
//        Hunger,
//        Sanity
//    };
//
//    class TabButton : public IScript
//    {
//    public:
//        void Start() override;
//        void Update(float dt) override;
//        void End() override {}
//
//        IScript* Clone() const override { return new TabButton(*this); }
//        std::string GetScriptName() override { return "TabButton"; }
//
//        void SetScriptEntityID(Entity& e) override { entityID = e; }
//        Entity& GetScriptEntityID() override { return entityID; }
//
//        //called by ButtonResponse
//        void SelectTab(HTP_Tab tab);
//
//    private:
//        Entity manualEntity{};
//        std::unordered_map<HTP_Tab, Entity> tabEntities;
//        std::unordered_map<Entity, float> baseX;
//
//        HTP_Tab currentTab = HTP_Tab::Overview;
//
//        float selectedOffsetX = -0.08f;
//    };
//}
