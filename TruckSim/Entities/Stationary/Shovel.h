#pragma once
#include <queue>

#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

class Shovel : public StationaryEntity
{
public:
    Shovel(int id_, int loadSpeed) : StationaryEntity(id_), loadSpeed(loadSpeed) {}

    int TrucksInQueue();
    int TimeToLoad();
    void EnqueueTruck(TruckId truck);
    void DequeueTruck(TruckId truck);
    TruckId GetFirst();
    
private:
    int loadSpeed = 0;
    std::queue<TruckId> waitingQueue;
};
