#pragma once
#include "../Entity.h"

class MobileEntity : public Entity
{
public:
    MobileEntity(int id, int speed) : Entity(id) {} // TODO make a sort of construction DTO so we can create an entity from a config
    
private:
    float speed = 0;
};
