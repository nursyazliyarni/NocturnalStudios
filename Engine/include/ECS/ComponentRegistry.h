/*!------------------------------------------------------------------------------------------------
	File: ComponentRegistry.h
	Project: NocturnalStudio

	Brief:
		This file contains the base class which has functions used to serialize and deserialize 
        components for ECS.It also allows them to be displayed in the editor

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the ComponentRegistry class

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
    /*!*************************************************************************
    The component registry makes it much easier for the various systems of the
    game engine to operate on componentsx
    ****************************************************************************/
    class ComponentRegistry {
    public:
        /*!*************************************************************************
        These are the fuctions that all components have in their base class declaration
        ****************************************************************************/
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

        /*!*************************************************************************
        This registers a component into the component registry's map. The user has
        to provide the registry the name they want the component to have
        ****************************************************************************/
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


            //Deserialize and add to entity
            desc.deserializeAndAddFunc = [this](Entity e, const rapidjson::Value& obj) -> bool {
                T component{};
                component.SetComponentEntityID(e);

                if (component.Deserialize(obj)) {
                    GetECS().template AddComponent<T>(e, component);
                    return true;
                }
                return false;
                };

            //serialize
            desc.serializeFunc = [this](Entity e, rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) -> bool {
                T& component{ GetECS().template GetComponent<T>(e) };
                return component.Serialize(writer);
             };

            //show in leveleditor
            desc.displayInEditor = [this](Entity e) {
                if (GetECS().template HaveComponent<T>(e))
                {
                    GetECS().template GetComponent<T>(e).ToEditor();
                }
                };

            mComponents.emplace(name, desc);
        }

        /*!*************************************************************************
        Returns a const reference to the component map
        ****************************************************************************/
        const std::unordered_map<std::string, ComponentDescriptor>& GetAll() const {
            return mComponents;
        }

    protected:
        ECS& GetECS();
        
        std::unordered_map<std::string, ComponentDescriptor> mComponents{};
    };
}
