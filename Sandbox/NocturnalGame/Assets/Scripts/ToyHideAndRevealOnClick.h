#pragma once
#include "ECS/ECS.h"
#include "Input/Input.h"
#include "Math/Vmath.h"
#include "Scripts/IScript.h"


namespace NocturnalEngine
{
    class ToyHideAndRevealOnClick : public IScript

    {
    public:
        ToyHideAndRevealOnClick();
        void Start() override;
        void Update(float dt) override;
        void End() override;

    private:
        void FindBG();
        vec2D RandomPointInsideBG() const;
        bool IsMouseNearHiddenSpot() const;

    private:
        InputSystem* mInput{ nullptr };

        Entity mBG{ 0 };
        bool mHasBG{ false };

        bool mHidden{ true };
        vec2D mHiddenPos{};
        vec2D mOriginalScale{};

        // Store original collider so we can disable it while hidden
        bool mHadCollider{ false };
        Collider mOriginalCollider{};

        float mRevealRadius{ 35.0f }; // tweak
    };
}
