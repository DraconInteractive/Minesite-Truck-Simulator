#pragma once
#include "MobileEntity.h"

class Truck : public MobileEntity
{
public:
    Truck(int id_, int speed, int capacity, int currentLoad) : MobileEntity(id_, speed), capacity(capacity), currentLoad(currentLoad) {} // TODO make a sort of construction DTO so we can create an entity from a config

private:
    int capacity = 0;
    int currentLoad = 0;
};
