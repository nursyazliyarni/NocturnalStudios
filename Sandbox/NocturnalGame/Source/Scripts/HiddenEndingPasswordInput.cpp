#include "pch.h"
#include "Scripts/HiddenEndingPasswordInput.h"
#include "ECS/Components/Text.h"
#include "ECS/SceneManager.h"
#include "Input/Input.h"
#include <GLFW/glfw3.h>
#include <array>
#include <cctype>
#include <set>

namespace NocturnalEngine
{
    namespace
    {
        using KeyMapping = std::pair<int, char>;

        constexpr std::array<KeyMapping, 26> kSupportedKeys{ {
            { GLFW_KEY_A, 'A' }, { GLFW_KEY_B, 'B' }, { GLFW_KEY_C, 'C' }, { GLFW_KEY_D, 'D' },
            { GLFW_KEY_E, 'E' }, { GLFW_KEY_F, 'F' }, { GLFW_KEY_G, 'G' }, { GLFW_KEY_H, 'H' },
            { GLFW_KEY_I, 'I' }, { GLFW_KEY_J, 'J' }, { GLFW_KEY_K, 'K' }, { GLFW_KEY_L, 'L' },
            { GLFW_KEY_M, 'M' }, { GLFW_KEY_N, 'N' }, { GLFW_KEY_O, 'O' }, { GLFW_KEY_P, 'P' },
            { GLFW_KEY_Q, 'Q' }, { GLFW_KEY_R, 'R' }, { GLFW_KEY_S, 'S' }, { GLFW_KEY_T, 'T' },
            { GLFW_KEY_U, 'U' }, { GLFW_KEY_V, 'V' }, { GLFW_KEY_W, 'W' }, { GLFW_KEY_X, 'X' },
            { GLFW_KEY_Y, 'Y' }, { GLFW_KEY_Z, 'Z' }
        } };
    }

    std::string HiddenEndingPasswordInput::sCurrentInput{};
    std::string HiddenEndingPasswordInput::sLastSubmittedPassword{};
    bool HiddenEndingPasswordInput::sHasSubmittedPassword{ false };

    void HiddenEndingPasswordInput::Start()
    {
        mCurrentInput.clear();
        sCurrentInput.clear();
        ResetSubmittedPassword();
        RefreshText();
        RefreshHintVisibility();
    }

    void HiddenEndingPasswordInput::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);

        if (!p_ecs.HaveComponent<Text>(entityID))
            return;

        if (p_Input->KeyPressed(GLFW_KEY_BACKSPACE) && !mCurrentInput.empty())
        {
            mCurrentInput.pop_back();
        }

        for (const auto& [key, character] : kSupportedKeys)
        {
            if (p_Input->KeyPressed(key))
            {
                AppendCharacter(character);
            }
        }

        if (p_Input->KeyPressed(GLFW_KEY_ENTER) || p_Input->KeyPressed(GLFW_KEY_KP_ENTER))
        {
            sLastSubmittedPassword = mCurrentInput;
            sHasSubmittedPassword = true;
            if (mCurrentInput == "HELP")
            {
                p_Scene->setSceneToLoad("Assets/Scene/goodEndingScene.json");
            }
        }

        sCurrentInput = mCurrentInput;

        RefreshText();
        RefreshHintVisibility();
    }

    void HiddenEndingPasswordInput::ResetSubmittedPassword()
    {
        sCurrentInput.clear();
        sLastSubmittedPassword.clear();
        sHasSubmittedPassword = false;
    }

    void HiddenEndingPasswordInput::RefreshText() const
    {
        if (!p_ecs.HaveComponent<Text>(entityID))
            return;

        auto& text = p_ecs.GetComponent<Text>(entityID);
        text.SetContent(mCurrentInput);
    }

    void HiddenEndingPasswordInput::RefreshHintVisibility() const
    {
        const Entity hintEntity = FindEntityByName("PasswordHint");
        if (hintEntity == MAX_ENTITIES || !p_ecs.HaveComponent<Text>(hintEntity))
            return;

        p_ecs.GetComponent<Text>(hintEntity).SET_visibility(mCurrentInput.empty());
    }

    void HiddenEndingPasswordInput::AppendCharacter(char character)
    {
        if (static_cast<int>(mCurrentInput.size()) >= maxLength)
            return;

        if (forceUppercase)
        {
            character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }

        mCurrentInput.push_back(character);
    }

    Entity HiddenEndingPasswordInput::FindEntityByName(const std::string& name) const
    {
        const std::set<Entity> entities{ p_ecs.GetAliveEntitiesSet() };
        for (Entity entity : entities)
        {
            if (p_ecs.HaveComponent<NameTag>(entity) &&
                p_ecs.GetComponent<NameTag>(entity).GetNameTag() == name)
            {
                return entity;
            }
        }

        return MAX_ENTITIES;
    }
}
