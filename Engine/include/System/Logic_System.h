#pragma once
#include <iostream>
#include "logic/Animal_Logic.h"
#include "ECS/ECS.h"

struct AnimalLogicComponent {
    //Animal_Logic logic;
};

// ==================== ECS SYSTEM CLASS ====================

// Forward declare ECS to avoid circular includes
class ECS;

class Logic_System {
public:
    void update(float deltaTime);

private:
    ECS& _ecs_;
};
