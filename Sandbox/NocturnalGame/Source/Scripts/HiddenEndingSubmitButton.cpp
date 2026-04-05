#include "pch.h"
#include "Scripts/HiddenEndingSubmitButton.h"
#include "Scripts/HiddenEndingPasswordInput.h"
#include "GUI/GUI.h"
#include "ECS/SceneManager.h"
#include <GLFW/glfw3.h>

namespace NocturnalEngine
{
    HiddenEndingSubmitButton::HiddenEndingSubmitButton()
        : mInput{ InputSystem::GetInstance() }
    {
    }

    void HiddenEndingSubmitButton::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (mInput->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            mPressedInside = IsMouseOver(entityID);
        }

        if (mInput->MouseIsReleased(GLFW_MOUSE_BUTTON_LEFT))
        {
            const bool releasedInside = IsMouseOver(entityID);
            if (mPressedInside && releasedInside &&
                HiddenEndingPasswordInput::GetCurrentInput() == "HELP")
            {
                p_Scene->setSceneToLoad("Assets/Scene/goodEndingScene.json");
            }

            mPressedInside = false;
        }
    }

    bool HiddenEndingSubmitButton::IsMouseOver(Entity entity) const
    {
        if (!p_ecs.HaveComponent<Transform>(entity) || !p_ecs.HaveComponent<Collider>(entity))
            return false;

        vec2D mousePos{ p_GUI->MousePosition.x, p_GUI->MousePosition.y };
        auto& transform = p_ecs.GetComponent<Transform>(entity);
        auto& collider = p_ecs.GetComponent<Collider>(entity);

        for (int i = 0; i < 2; ++i)
        {
            vec2D offsetPos{
                transform.GetWorldPos().x + collider[i].mOffset.x,
                transform.GetWorldPos().y + collider[i].mOffset.y
            };

            switch (collider[i].mCol)
            {
            case Collider::ColliderType::circle:
            case Collider::ColliderType::bubble:
                if (distance(mousePos, offsetPos) < collider[i].mRadius)
                    return true;
                break;

            case Collider::ColliderType::rect:
            case Collider::ColliderType::box:
            {
                vec2D minPt{ offsetPos.x - collider[i].mMin.x, offsetPos.y - collider[i].mMin.y };
                vec2D maxPt{ offsetPos.x + collider[i].mMax.x, offsetPos.y + collider[i].mMax.y };
                if (mousePos.x > minPt.x && mousePos.x < maxPt.x &&
                    mousePos.y > minPt.y && mousePos.y < maxPt.y)
                {
                    return true;
                }
                break;
            }
            }
        }

        return false;
    }
}
