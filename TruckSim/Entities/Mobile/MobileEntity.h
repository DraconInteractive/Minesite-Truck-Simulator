#pragma once
#include "../Entity.h"

class MobileEntity : public Entity
{
public:
    MobileEntity(int id_) : Entity(id_) {} // TODO make a sort of construction DTO so we can create an entity from a config
    
    virtual float GetSpeed() const = 0;
};
