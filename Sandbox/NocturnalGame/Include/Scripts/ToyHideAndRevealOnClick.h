
#include "ECS/ECS.h"
#include "Input/Input.h"
#include "Math/Vmath.h"
#include "Scripts/IScript.h"
#include <random>
#include "ECS/Components/AnimalAttributes.h"
#include "ECS/Components/ItemsAttributes.h"


namespace NocturnalEngine
{
    // Toy behavior:
    // - First scene entry: toy starts visible
    // - Later entries: toy can start hidden at a stored random position
    // - Hidden toy reveals when player clicks inside toy hit area
    class ToyHideAndRevealOnClick : public IScript

    {
    public:
        // Gets input system reference used for mouse click checks.
        ToyHideAndRevealOnClick();

        // Called when script starts for this entity.
        void Start() override;

        // Called every frame; handles reveal click while hidden.
        void Update(float dt) override;

        // Called when leaving scene; stores next hidden position.
        void End() override;

        // This script is allowed on any entity (runtime checks still apply).
        bool Requirements(Entity e) override {
            UNREFERENCED_PARAMETER(e);
            return true;
        }

        // Script type name used by registry/editor.
        virtual std::string GetScriptName() override
        {
            return "ToyHideAndRevealOnClick";
        }

        // Creates a copy used by scripting system.
        virtual ToyHideAndRevealOnClick* Clone() const override
        {
            return new ToyHideAndRevealOnClick(*this);
        }

        // ECS script binding helpers.
        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }
    private:
        bool CheckHide(AnimalAttributes& aa);
        int HideToy();        
        void IsMouseNearHiddenSpot(ItemsAttributes& iA, AnimalAttributes& aa);
        void ResetThing(ItemsAttributes& iA, AnimalAttributes& aa, Transform& t, Sprite& s);
        vec2D GetRandomPointInBush(Entity bushEntity) const;

    private:
        // Finds environment entity used as hide-area bounds.
        void FindBG();

        // Returns random point inside environment bounds for hiding toy.
        vec2D RandomPointInsideBG();

        // Returns toy half-size used for bounds-safe placement.
        vec2D GetToyHalfExtents();

        // Returns true when mouse click is inside hidden toy hit area.
        bool IsMouseNearToy();

        // Builds stable key for per-toy runtime state map.
        std::string BuildToyKey() const;

        // Applies hidden state: move to hide position, scale 0, disable collider.
        void ApplyHiddenState();

        // Applies visible state: restore scale and collider.
        void ApplyVisibleState();

    private:
        // Input system for mouse click checks.
        InputSystem* mInput{ nullptr };
        float checkPeriod{};

        vec2D ogPos{};
        int ogLayer{ 0 };
        Entity animalEntity{MAX_ENTITIES};
        std::vector<Entity> hidingSpots{};
        bool hasBeenHidden{ false };

    };
}
