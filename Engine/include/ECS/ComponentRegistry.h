/*!------------------------------------------------------------------------------------------------
	File: ComponentRegistry.h
	Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize name tag components for ECS.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the draggable class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#pragma once
#include "pch.h"
#include "Components/IComponent.h"
#include "ECS/Types.h"
#include "ECS.h"
#include "Serialization/JSONserialization.h"

namespace NocturnalEngine {
    class ECS;
    class ComponentRegistry {
    public:
        struct ComponentDescriptor {
            std::string name{};

            // Add component to entity
            std::function<void(Entity)> addFunc{};

            // Remove component from entity
            std::function<void(Entity)> removeFunc{};

            // Check if entity has component
            std::function<bool(Entity)> hasFunc{};

            //Deserialize and add in one step
            std::function<bool(Entity, const rapidjson::Value&)> deserializeAndAddFunc{};

            //Serialize
            std::function<bool(Entity, rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer)> serializeFunc{};

            //Editor Display func
            std::function<void(Entity)> displayInEditor{};
        };

        ComponentRegistry() = default;

        template<typename T>
        void RegisterComponent(const std::string& name) {
            ComponentDescriptor desc;
            desc.name = name;

            // Add component
            desc.addFunc = [this](Entity e) {
                if (!GetECS().template HaveComponent<T>(e))
                    GetECS().template AddComponent<T>(e, T{});
                };

            // Remove component
            desc.removeFunc = [this](Entity e) {
                if (GetECS().template HaveComponent<T>(e))
                    GetECS().template RemoveComponent<T>(e);
                };

            // Check if has component
            desc.hasFunc = [this](Entity e) {
                return GetECS().template HaveComponent<T>(e);
                };


            //Deserialize and add in one step 
            desc.deserializeAndAddFunc = [this](Entity e, const rapidjson::Value& obj) -> bool {
                T component{};
                component.SetComponentEntityID(e);

                if (component.Deserialize(obj)) {
                    GetECS().template AddComponent<T>(e, component);
                    return true;
                }
                return false;
                };

            //Deserialize and add in one step 
            desc.serializeFunc = [this](Entity e, rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) -> bool {
                T& component{ GetECS().template GetComponent<T>(e) };
                return component.Serialize(writer);
             };

            desc.displayInEditor = [this](Entity e) {
                if (GetECS().template HaveComponent<T>(e))
                {
                    GetECS().template GetComponent<T>(e).ToEditor();
                }
                };

            mComponents[name] = desc;
        }

        const std::map<std::string, ComponentDescriptor>& GetAll() const {
            return mComponents;
        }

    protected:
        ECS& GetECS();
        
        std::map<std::string, ComponentDescriptor> mComponents{};
    };
}
