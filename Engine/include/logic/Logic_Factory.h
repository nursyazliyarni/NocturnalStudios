#pragma once

/*!------------------------------------------------------------------------------------------------
    File: Logic_Factory.h
    Project: NocturnalStudio

    This file contains the functions needed to register all the current available game logic for
    our animal game objects.

    Primary Author:
        Yu Xuan
        - Implemented functions to create Logic_Factory,add logic,create logic and get logic

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

/*
Logic_Factory is responsible for:
1)Knowing what logic scripts exist (FlamingoLogic, HorseLogic, OwlLogic, etc.)
2)Creating instances of those logic classes by name.
3)Providing a list of all registered logic scripts.
*/

#include <functional>
#include <memory>
#include "logic/Base_Logic.h"

class Logic_Factory {
public:
    //LogicCreator is a function type that takes no arguments and returns std::unique_ptr<Base_Logic>()
    using LogicCreator = std::function<std::unique_ptr<Base_Logic>()>;

    static Logic_Factory& get_instance(); //ONLY 1 logic factory which is accessible everywhere
    void add_logic(const std::string& name, const LogicCreator& creator); //adds a new logic type to the logic factory

    const std::unique_ptr<Base_Logic>& create_logic(const std::string& name); //creates logic object
    std::vector<std::string> get_existing_logics();

private:
    std::unordered_map<std::string, LogicCreator> logic_creators;
};
