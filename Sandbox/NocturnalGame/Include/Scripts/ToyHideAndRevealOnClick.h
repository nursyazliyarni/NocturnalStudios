
#include "ECS/ECS.h"
#include "Input/Input.h"
#include "Math/Vmath.h"
#include "Scripts/IScript.h"
#include <random>


namespace NocturnalEngine
{
    class ToyHideAndRevealOnClick : public IScript

    {
    public:
        ToyHideAndRevealOnClick();
        void Start() override;
        void Update(float dt) override;
        void End() override;
        bool Requirements(Entity e) override {
            UNREFERENCED_PARAMETER(e);
            return true;
        }
        virtual std::string GetScriptName() override
        {
            return "ToyHideAndRevealOnClick";
        }

        virtual ToyHideAndRevealOnClick* Clone() const override
        {
            return new ToyHideAndRevealOnClick(*this);
        }
        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }

        virtual Entity& GetScriptEntityID() override { return entityID; }
    private:
        void FindBG();
        vec2D RandomPointInsideBG();
        vec2D GetToyHalfExtents();
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
        std::mt19937 mRng{ std::random_device{}() };

        float mRevealRadius{ 35.0f }; // tweak
    };
}
