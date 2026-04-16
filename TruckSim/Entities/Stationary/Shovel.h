#pragma once
#include <queue>

#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

struct ShovelId
{
    int value = -1;
};

class Shovel : public StationaryEntity
{
public:
    Shovel(int id_, Position _position, double loadSpeed) : StationaryEntity(id_, _position), loadSpeed(loadSpeed) {}

    ShovelId GetShovelId() const
    {
        return ShovelId{GetId()};
    }
    
    int TrucksInQueue() const
    {
        return static_cast<int>(waitingQueue.size());
    }
    
    double TimeToLoad() const
    {
        return loadSpeed; // TODO change this to take in a truck parameter, and use truck current load / speed
    }
    
    void EnqueueTruck(TruckId truck)
    {
        std::cout << "Adding truck " << truck.value << " to queue at shovel " << id << "\n";
        waitingQueue.push(truck);
    }
    
    void DequeueTruck()
    {
        std::cout << "Removing truck from queue at shovel " << id << "\n";
        waitingQueue.pop();
    }
    
    TruckId GetFirst() const
    {
        return waitingQueue.front();
    }

    static ShovelId GetBestShovel(Truck& truck, std::vector<Shovel>& shovels, float distPriority = 1, float queuePriority = 1);
    
private:
    double loadSpeed = 0;
    std::queue<TruckId> waitingQueue;
};
