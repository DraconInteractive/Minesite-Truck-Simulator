#pragma once
#include <queue>
#include "StationaryEntity.h"
#include "../Mobile/Truck.h"

class Dump : public StationaryEntity
{
public:
    Dump(int id_) : StationaryEntity(id_){}

    int TrucksInQueue();
    int TimeToDump();
    void EnqueueTruck(TruckId truck);
    void DequeueTruck(TruckId truck);
    TruckId GetFirst();

private:
    int dumpSpeed = 0;
    std::queue<TruckId> waitingQueue;
};
