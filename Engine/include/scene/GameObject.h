/*****************************************************************//**
 * \file	GameObject.h
 * \brief	This header file contains the declarations for
 *			the Engine's GameObjects Class
 * 
 * 
 * \author	Laycock Enhui Sarah (e.laycock@digipen.edu)
 * \date	September 2025
 *********************************************************************/

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <iostream>
#include <string>
//#include <vector>
#include <utility>
#include "./math/Matrix.h"
#include "./scene/Component.h"

// [========================================*========================================]
//  Base GameObject Class
// [========================================*========================================]
class GameObject {
private:
	//id & Name
	size_t id{ };
	std::string name{ " " };

	//flags to denote activity
	bool active{ false }, enabled{ false };

	//Other Components
	std::vector<std::unique_ptr<Component>> components{};

	//Parent Obj.. If applicable
	GameObject* parent_object{ nullptr };

public:

	// [====================*====================]
	//  Constructors & Destructors
	// [====================*====================]
	// Default Constructor - rule of zero
	GameObject()=default;
	GameObject(std::string);

	GameObject(const GameObject&);


	~GameObject();

	std::unique_ptr<GameObject> Clone() const;

	// [====================*====================]
	//  Component Attachment
	// [====================*====================]

	void Attach(std::unique_ptr<Component>&&);

	template<typename T, typename... Args>
	T* Attach(Args&&... args) {
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		Component* comp = std::make_unique<T>(std::forward<Args>(args)...);
		comp->SetOwner(this);
	
		T* ptr = static_cast<T*>(comp.get());
		components.push_back(std::move(comp));
		return ptr;
	}

	// [====================*====================]
	//  Lifecycle Functions
	// [====================*====================]
	virtual void Start();

	virtual void Update();

	virtual void Render(Renderer&);

	virtual void Kill();


	// [====================*====================]
	//  Transform Handling Functions
	// [====================*====================]
	//void Pos(const Vector3D& pos);
	//Vector3D Pos() const;

	//void Rot(const Matrix<4>& rot);
	//Matrix<4> Rot() const;

	//void Scale(const Vector3D& scale);
	//Vector3D Scale() const;

	//Matrix<4> Transform() const;


	// [====================*====================]
	//  Getters and Setters
	// [====================*====================]
	size_t ID()const;
	void set_ID(size_t i) { id = i; }

	void Activate();

	bool ActiveStatus() const;
	void ActiveStatus(bool);

	bool EnabledStatus() const;
	void EnabledStatus(bool);


	// [====================*====================]
	//  Parenting
	// [====================*====================]
	GameObject* Parent() const;
	bool MakeParent(GameObject*);
	void ClearParent();
	//do i add children?

	// [====================*====================]
	//  Print & Load
	// [====================*====================]
	virtual void Print(std::ostream&) const;
	virtual void Load(std::istream&);

	// [====================*====================]
	//  Print & Load
	// [====================*====================]
	

};
std::istream& operator>>(std::istream& in, GameObject& obj);
std::ostream& operator<<(std::ostream& out, const GameObject& obj);
namespace GameObjects{ 
	void GameObject_test(); 
}
#endif
