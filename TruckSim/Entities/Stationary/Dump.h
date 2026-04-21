#pragma once
#include <queue>
#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

#include "../EntityIdTypes.h"

struct SimState;

class Dump : public StationaryEntity
{
public:
    Dump(int id_, Position _position, float dumpSpeed) : StationaryEntity(id_, _position), dumpSpeed(dumpSpeed){}

    DumpId GetDumpId() const
    {
        return DumpId{GetId()};
    }
    
    int TrucksInQueue() const
    {
        return static_cast<int>(waitingQueue.size());
    }
    
    float TimeToDump(const Truck& truck) const
    {
        return truck.CurrentLoad() / dumpSpeed;
    }
    
    void EnqueueTruck(TruckId truck)
    {
        std::cout << "Adding truck " << truck.value << " to queue at dump " << id << "\n";
        waitingQueue.push_back(truck);
    }
    
    void DequeueTruck()
    {
        std::cout << "Remove truck from queue at dump " << id << "\n";
        waitingQueue.pop_front();
    }
    
    TruckId GetFirst() const
    {
        return waitingQueue.front();
    }

    const std::deque<TruckId>& GetQueue() const
    {
        return waitingQueue;
    }
    
    static DumpId GetBestDump(const SimState& sim, const Truck& truck, float travelTimePriority = 1, float queuePriority = 1);

private:
    // Time per unit, for now say tonnes per minute
    float dumpSpeed = 0;
    std::deque<TruckId> waitingQueue;
};
