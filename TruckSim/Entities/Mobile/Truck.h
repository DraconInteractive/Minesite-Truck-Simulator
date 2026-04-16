#pragma once
#include "MobileEntity.h"

enum class TruckState
{
    Idle,
    Travelling,
    Loading,
    Queueing,
    Dumping
};

struct TruckId
{
    int value = -1; // Default to invalid ID
};

class Truck : public MobileEntity
{
public:
    Truck(int id_, int speed, int capacity, int currentLoad) : MobileEntity(id_, speed), capacity(capacity), currentLoad(currentLoad) {} // TODO make a sort of construction DTO so we can create an entity from a config

    TruckState GetState();
    void SetState(TruckState newState);
    
private:
    int capacity = 0;
    int currentLoad = 0;
    TruckState state = TruckState::Idle;
};
