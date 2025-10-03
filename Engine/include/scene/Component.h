/*****************************************************************//**
 * \file	GameObject.h
 * \brief	This header file contains the class dleclerations
 *			For a component class 
 *
 *
 * \author	Laycock Enhui Sarah (e.laycock@digipen.edu)
 * \date	September 2025
 *********************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <rapidjson/document.h>
#include "graphics/Renderer.h"

enum COMPONENT_TYPE {
    NONE = 0,
    TRANSFORM = 1,
	RENDERING =2,
	COLLISION =3,
	//PHYSICS=3,
	LOGIC=4
};
class GameObject;

//base component object
class Component {
protected:
	COMPONENT_TYPE type{ NONE };
	std::string c_name{ "BasicComponent" };
	GameObject* owner{ nullptr };
public:
	Component() = default;
	Component(COMPONENT_TYPE t,std::string s);
    Component(const Component&);
    virtual ~Component() = default; // Ensure virtual destructor
    virtual std::unique_ptr<Component> Clone() const=0;
	void SetOwner(GameObject* o);
	virtual void Render(Renderer&);
    virtual void Transform();
	virtual bool Collide();
	virtual void Logic();
    std::string ComponenetName() const;
    COMPONENT_TYPE Type() const;
};

//Transform Components


//Renderer Components
class CircleRenderer :public Component {
private:
	float radius{ 1.f };
public:
	CircleRenderer(float r=1.f);
    CircleRenderer(const rapidjson::Value& j);
    CircleRenderer(const CircleRenderer&);
    std::unique_ptr<Component> Clone() const;
	void Render(Renderer&) override;
};

class RectRenderer :public Component {
private:
	float width{ 1.f }, height{ 1.f };
public:
	RectRenderer(float w=1.f,float h=1.f);
    RectRenderer(const rapidjson::Value& j);
    RectRenderer(const RectRenderer&);
    std::unique_ptr<Component> Clone() const;
	void Render(Renderer&) override;

};

class SpriteRenderer :public Component {
private:
	std::string sprite_path{};
public:
	SpriteRenderer(std::string="");
    SpriteRenderer(const rapidjson::Value& j);
    SpriteRenderer(const SpriteRenderer&);
    std::unique_ptr<Component> Clone() const;
	void Render(Renderer&) override;
    
};

//Collision Components
class AABBCollider :public Component {
private:
	float bounding_box_size{1.f};
public:
	AABBCollider(float bbsize=1.f);
    AABBCollider(const rapidjson::Value& j);
    AABBCollider(const AABBCollider&);
    std::unique_ptr<Component> Clone() const;
	bool Collide() override;
};

//Logic Components
class AnimalBehaviour :public Component {
private:
	std::string sound{ "moo" };
public:
	AnimalBehaviour(std::string s="");
    AnimalBehaviour(const rapidjson::Value& j);
    AnimalBehaviour(const AnimalBehaviour&);
    std::unique_ptr<Component> Clone() const;
	void Logic() override;
};



class ComponentFactory {
public:
    using Json = rapidjson::Value;
    using Creator = std::function<std::unique_ptr<Component>(const Json&)>;

    // Register a creator for a name (overwrites if exists)
    static void Register(const std::string& name, Creator creator) {
        auto& map = GetMap();
        map[name] = std::move(creator);
    }

    // Convenience: register a concrete type T that exposes T::T(const Json&)
    template<typename T>
    static void RegisterType(const std::string& name) {
        Register(name, [](const Json& j) {
            return std::unique_ptr<Component>(new T(j));
            });
    }

    // Create or throw
    static std::unique_ptr<Component> Create(const std::string& name, const Json& j) {
        auto& map = GetMap();
        auto it = map.find(name);
        if (it == map.end()) throw std::runtime_error("Unknown component: " + name);
        return it->second(j);
    }

    // TryCreate - returns nullptr when unknown
    static std::unique_ptr<Component> TryCreate(const std::string& name, const Json& j) {
        auto& map = GetMap();
        auto it = map.find(name);
        if (it == map.end()) return nullptr;
        return it->second(j);
    }

private:
    // function-local static ensures initialization when first used (safe across TUs)
    static std::unordered_map<std::string, Creator>& GetMap() {
        static std::unordered_map<std::string, Creator> map;
        return map;
    }
};


#endif
