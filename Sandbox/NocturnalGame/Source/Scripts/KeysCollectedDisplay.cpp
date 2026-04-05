/*!------------------------------------------------------------------------------------------------
    File: KeysCollectedDisplay.cpp
    Project: NocturnalStudios

    Date:
        2026-04-05

    Brief:
        Updates the UI that displays the number of collected encryption letters.

    Primary Author:
         Nur Syazliyarni (100%)
        - Added the file header.
        - Email: nursyazliyarni.b

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
