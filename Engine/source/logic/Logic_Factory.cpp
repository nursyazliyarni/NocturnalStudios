/*!------------------------------------------------------------------------------------------------
    File: Logic_Factory.cpp
    Project: NocturnalStudio

    This file contains the functions needed to register all the current available game logic for
    our animal game objects.

    Primary Author:
        Yu Xuan
        - Implemented functions to create Logic_Factory,add logic,create logic and get logic

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "logic/Logic_Factory.h"

Logic_Factory& Logic_Factory::get_instance(){
    static Logic_Factory logic_factory;
    return logic_factory;
}

void Logic_Factory::add_logic(const std::string& logic_name, const LogicCreator& creator){
    logic_creators[logic_name] = creator;
}

const std::unique_ptr<Base_Logic>& Logic_Factory::create_logic(const std::string& logic_name) {
    auto it = logic_creators.find(logic_name);
    if (it != logic_creators.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> Logic_Factory::get_existing_logics() {
    std::vector<std::string> temp_vector;
    for (const auto& pair : logic_creators) {
        temp_vector.push_back(pair.first);
    }
    return temp_vector;
}