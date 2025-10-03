#ifndef SYSTEM_H
#define SYSTEM_H

/*!------------------------------------------------------------------------------------------------
	File: System.h
	Project: NocturnalStudio

	This file contains the class definition of a System. It is an abstract base class 
	that provides the common interface for all system in the ECS architecture.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the System class and its interface.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include <string>

class ECS;

class System {
public:
	System(ECS& ecs, std::string name) : ECSRegistry{ ecs }, systemName{ name } {}
	virtual ~System() = 0 {}
	virtual void update(double deltaTime) = 0;
	std::string name() const { return systemName; }

protected:
	ECS& ECSRegistry;
	std::string systemName;
};

#endif // SYSTEM_H