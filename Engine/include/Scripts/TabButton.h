
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

#pragma once
#include "Scripts/IScript.h"
#include <glm/vec2.hpp>

namespace NocturnalEngine
{
    struct ButtonBB
    {
        glm::vec2 min;
        glm::vec2 max;
    };

    class TabButton : public IScript
    {
    public:
        TabButton();
        ~TabButton() = default;

        void Start() override;
        void Update(float Frametime) override;
        void End() override;
        IScript* Clone() const override;
        std::string GetScriptName() override;

        void SetScriptEntityID(Entity& e) override { entityID = e; }
        Entity& GetScriptEntityID() override { return entityID; }

        void SetBoundingBox(const ButtonBB& bb) { mBB = bb; }
        bool IsSelected() const { return mSelected; }

    private:
        bool mSelected;
        ButtonBB mBB;

        bool IsMouseOver(const glm::vec2& mouse) const;
    };
}