#pragma once
#include <queue>
#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

class Dump : public StationaryEntity
{
public:
    Dump(int id_, Position _position, double dumpSpeed) : StationaryEntity(id_, _position), dumpSpeed(dumpSpeed){}

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

private:
    double dumpSpeed = 0;
    std::queue<TruckId> waitingQueue;
};
