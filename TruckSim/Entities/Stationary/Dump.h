#pragma once
#include <queue>
#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

struct DumpId
{
    int value = -1;
};

class Dump : public StationaryEntity
{
public:
    Dump(int id_, Position _position, double dumpSpeed) : StationaryEntity(id_, _position), dumpSpeed(dumpSpeed){}

    DumpId GetDumpId() const
    {
        return DumpId{GetId()};
    }
    
    int TrucksInQueue() const
    {
        return static_cast<int>(waitingQueue.size());
    }
    
    double TimeToDump() const
    {
        return dumpSpeed; // TODO change this to take in a truck parameter, and use truck current load / speed
    }
    
    void EnqueueTruck(TruckId truck)
    {
        std::cout << "Adding truck " << truck.value << " to queue at dump " << id << "\n";
        waitingQueue.push(truck);
    }
    
    void DequeueTruck()
    {
        std::cout << "Remove truck from queue at dump " << id << "\n";
        waitingQueue.pop();
    }
    
    TruckId GetFirst() const
    {
        return waitingQueue.front();
    }

    static DumpId GetBestDump(Truck& truck, std::vector<Dump>& dumps, float distPriority = 1, float queuePriority = 1);

private:
    double dumpSpeed = 0;
    std::queue<TruckId> waitingQueue;
};
