#pragma once
#include <queue>

#include "../Entity.h"
#include "../Mobile/Truck.h"
#include "../EntityIdTypes.h"

struct SimState;

class StationaryEntity : public Entity
{
public:
    StationaryEntity(const int id_, const Position _position, const float processSpeed_) : Entity(id_)
    {
        SetPosition(_position);
        processSpeed = processSpeed_;
    }

    int TrucksInQueue() const
    {
        return static_cast<int>(waitingQueue.size());
    }
    
    virtual float TimeToProcess(const Truck& truck) const = 0;
    virtual bool IsDestination(const Event& evt) const = 0;
    
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

    const std::deque<TruckId>& GetQueue() const
    {
        return waitingQueue;
    }

    template<typename TSite>
    static int GetBestSite(const SimState& sim, const std::vector<TSite>& sites, const Truck& truck, float travelTimePriority, float queuePriority);

    void SetClosestNode(int nodeId)
    {
        closestNodeId = nodeId;
    }

    int GetClosestNodeId() const
    {
        return closestNodeId;
    }

protected:
    // Time per unit (define actual metric later. For now, say tonnes per minute?)
    float processSpeed = 0;
    std::deque<TruckId> waitingQueue;
    int closestNodeId = -1;
};
