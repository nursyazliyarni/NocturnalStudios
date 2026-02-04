#include "pch.h"
#include "Scripts/TabButton.h"
#include "GUI/GUI.h"
#include "Input/Input.h"
#include "GLFW/glfw3.h"

namespace NocturnalEngine
{
    TabButton::TabButton()
        : mSelected(false)
    {
    }

    void TabButton::Start()
    {
        mSelected = false;
    }

    void TabButton::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        auto& gui = gui_system::GetInstance();
        auto& input = InputSystem::GetInstance();

        const glm::vec2& mousePos = gui->MousePosition;

        if (input->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            mSelected = IsMouseOver(mousePos);
        }
    }

    void TabButton::End() {}

    IScript* TabButton::Clone() const
    {
        return new TabButton(*this);
    }

    std::string TabButton::GetScriptName()
    {
        return "TabButton";
    }

    bool TabButton::IsMouseOver(const glm::vec2& mouse) const
    {
        return (mouse.x >= mBB.min.x && mouse.x <= mBB.max.x &&
            mouse.y >= mBB.min.y && mouse.y <= mBB.max.y);
    }
}
