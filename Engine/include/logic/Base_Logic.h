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