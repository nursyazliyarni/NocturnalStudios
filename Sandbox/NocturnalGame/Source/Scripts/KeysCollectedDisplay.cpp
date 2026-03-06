#include "pch.h"
#include "Scripts/KeysCollectedDisplay.h"
#include "Scripts/EncryptionKey.h"
#include "ECS/Components/Text.h"

namespace NocturnalEngine
{
    void KeysCollectedDisplay::Start()
    {
        RefreshText();
    }

    void KeysCollectedDisplay::Update(float Frametime)
    {
        UNREFERENCED_PARAMETER(Frametime);
        RefreshText();
    }

    void KeysCollectedDisplay::RefreshText() const
    {
        if (!p_ecs.HaveComponent<Text>(entityID))
            return;

        auto& text = p_ecs.GetComponent<Text>(entityID);
        const size_t count = EncryptionKey::GetCollectedCount();
        const size_t maxKeys = EncryptionKey::GetMaxKeys();
        text.SetContent(std::to_string(count) + "/" + std::to_string(maxKeys));
    }
}
