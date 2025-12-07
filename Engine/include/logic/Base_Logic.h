/*!------------------------------------------------------------------------------------------------
	File: Base_Logic.h
	Project: NocturnalStudio

	This file contains the base implementation of our base logic

	Primary Author:
		Yu Xuan
		- Implemented base function of logic class

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include <string>

class Base_Logic {
public:
    virtual ~Base_Logic() = default; //to prevent possible memory leak

    virtual void initialization() {} //called once when the script is first attached to a GameObject

    virtual void cleanup() {} //called when the script is being destroyed

    virtual void update(float dt) = 0; //must be defined by child logic

    //virtual const std::string& get_logicNAME() const = 0; //must be defined by child logic
};