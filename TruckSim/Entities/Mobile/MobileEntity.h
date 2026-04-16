#pragma once
#include "../Entity.h"

class MobileEntity : public Entity
{
public:
    MobileEntity(int id_, float speed) : Entity(id_), speed(speed) {} // TODO make a sort of construction DTO so we can create an entity from a config
    
    float GetSpeed() const
    {
        return speed;
    }
    
private:
    float speed = 0;
};
