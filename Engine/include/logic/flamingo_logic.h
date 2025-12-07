#pragma once

#include "logic/Animal_Logic.h"

class FlamingoLogic : public Animal_Logic {
public:
    FlamingoLogic() = default;
    //void update(float dt) override;

    /*virtual const std::string& get_logicNAME() const {
        static const std::string name = "FlamingoLogic";
        return name;
    }*/

    // helper to set target entity (call after creation)
    void set_target(EntityID e) { target = e; }

private:
    EntityID target = INVALID_ENTITY_ID;
};
