#pragma once
#include <queue>

#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

#include "../EntityIdTypes.h"


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
    
    double TimeToLoad(const Truck& truck) const
    {
        return truck.RemainingCapacity() / loadSpeed;
    }
    
    void EnqueueTruck(TruckId truck)
    {
        std::cout << "Adding truck " << truck.value << " to queue at shovel " << id << "\n";
        waitingQueue.push_back(truck);
    }
    
    void DequeueTruck()
    {
        std::cout << "Removing truck from queue at shovel " << id << "\n";
        waitingQueue.pop_front();
    }
    
    TruckId GetFirst() const
    {
        return waitingQueue.front();
    }

    std::deque<TruckId> GetQueue() const
    {
        return waitingQueue;
    }
    
    static ShovelId GetBestShovel(Truck& truck, std::vector<Shovel>& shovels, float distPriority = 1, float queuePriority = 1);
    
private:
    // Time per unit (define actual metric later. For now, say tonnes per minute?)
    double loadSpeed = 0;
    std::deque<TruckId> waitingQueue;
};
