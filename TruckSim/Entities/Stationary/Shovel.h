#pragma once
#include <queue>

#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

#include "../EntityIdTypes.h"

struct SimState;

class Shovel : public StationaryEntity
{
public:
    Shovel(int id_, Position _position, float loadSpeed) : StationaryEntity(id_, _position, loadSpeed) {}

    ShovelId GetShovelId() const
    {
        return ShovelId{GetId()};
    }

    float TimeToProcess(const Truck& truck) const override
    {
        return truck.RemainingCapacity() / processSpeed;
    }

    bool IsDestination(const Event& evt) const override
    {
        return evt.shovel == GetShovelId();
    }
    
    static ShovelId GetBestShovel(const SimState& sim, const Truck& truck, float travelTimePriority = 1, float queuePriority = 1);
};
