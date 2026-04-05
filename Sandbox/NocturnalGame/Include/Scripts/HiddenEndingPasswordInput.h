#pragma once

#include "Scripts/IScript.h"
#include "ECS/Components/Text.h"

namespace NocturnalEngine
{
    class HiddenEndingPasswordInput : public IScript
    {
    public:
        HiddenEndingPasswordInput() = default;
        ~HiddenEndingPasswordInput() = default;

        virtual HiddenEndingPasswordInput* Clone() const override
        {
            return new HiddenEndingPasswordInput(*this);
        }

        virtual void Start() override;
        virtual void Update(float Frametime) override;
        virtual void End() override {}

        virtual std::string GetScriptName() override { return "HiddenEndingPasswordInput"; }

        bool Requirements(Entity e) override
        {
            return p_ecs.HaveComponent<Text>(e);
        }

        virtual void SetScriptEntityID(Entity& entity) override { entityID = entity; }
        virtual Entity& GetScriptEntityID() override { return entityID; }

        virtual std::vector<ScriptField> GetFields() override
        {
            return {
                { "MaxLength", &maxLength },
                { "ForceUppercase", &forceUppercase }
            };
        }

        static const std::string& GetCurrentInput() { return sCurrentInput; }
        static const std::string& GetLastSubmittedPassword() { return sLastSubmittedPassword; }
        static bool HasSubmittedPassword() { return sHasSubmittedPassword; }
        static void ResetSubmittedPassword();

    private:
        Entity FindEntityByName(const std::string& name) const;
        void RefreshText() const;
        void RefreshHintVisibility() const;
        void AppendCharacter(char character);

        std::string mCurrentInput;
        int maxLength{ 4 };
        bool forceUppercase{ true };

        static std::string sCurrentInput;
        static std::string sLastSubmittedPassword;
        static bool sHasSubmittedPassword;
    };
}
