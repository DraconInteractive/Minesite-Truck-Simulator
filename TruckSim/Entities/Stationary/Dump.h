#pragma once
#include <queue>
#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

#include "../EntityIdTypes.h"

struct SimState;

class Dump : public StationaryEntity
{
public:
    Dump(int id_, Position _position, float dumpSpeed) : StationaryEntity(id_, _position, dumpSpeed){}

    DumpId GetDumpId() const
    {
        return DumpId{GetId()};
    }

    float TimeToProcess(const Truck& truck) const override
    {
        return truck.CurrentLoad() / processSpeed;
    }

    bool IsDestination(const Event& evt) const override
    {
        return evt.dump == GetDumpId();
    }
    
    static DumpId GetBestDump(const SimState& sim, const Truck& truck, float travelTimePriority = 1, float queuePriority = 1);
};
