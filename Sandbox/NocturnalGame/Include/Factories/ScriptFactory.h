/*!------------------------------------------------------------------------------------------------
    File: ScriptFactory.h
    Project: NocturnalStudio

    Brief:
        This file enables injection of scripts into the engine side

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the ScriptFactory class and its rerspective functoiond


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "Scripts/IScriptRegistry.h"
#include "Scripts/IScript.h"


namespace NocturnalEngine{

    class ScriptFactory:public IScriptRegistry {
    public:
        //create a script
        using CreateFunc = std::function<IScript* ()>;
        using RequirementFunc = std::function<bool(Entity)>;
        // In ScriptFactory.h
        template<typename T>
        void RegisterScriptType() {
            T temp{};
            std::string name = temp.GetScriptName();

            RegisterScript(name,
                []() { return new T(); },
                [](Entity e) {
                    T temp;
                    return temp.Requirements(e);
                }
            );
        }
        //contains all info of a specific script
        struct ScriptDescriptor {
            std::string name;
            CreateFunc createFunc;
            RequirementFunc checkRequirements;
        };

        //singleton
        static ScriptFactory& Get() {
            static ScriptFactory instance;
            return instance;
        }

        void RegisterAllGameScripts();

        //register script
        void RegisterScript(const std::string& name, CreateFunc creator, RequirementFunc requirements) {
            mScripts[name] = { name, creator, requirements };
        }

        // Implement interface
        std::vector<std::string> GetAvailableScripts() const override {
            std::vector<std::string> names;
            for (std::pair<const std::string,ScriptDescriptor> scrp : mScripts)
                names.push_back(scrp.first);
            return names;
        }

        bool CanAttachScript(const std::string& name, Entity entity) const override {
            /*if (!p_ecs.HaveComponent<Transform>(entity)) {
                return false;
            }*/
            if (mScripts.count(name)) {
                return mScripts.at(name).checkRequirements(entity);
            }
            return false;
        }

        IScript* CreateScript(const std::string& name) override {
            if (mScripts.count(name))
                return mScripts[name].createFunc();
            return nullptr;
        }

    private:
        std::map<std::string, ScriptDescriptor> mScripts;
    };
}
