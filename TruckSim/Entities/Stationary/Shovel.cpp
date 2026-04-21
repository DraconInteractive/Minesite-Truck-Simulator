#include "Shovel.h"
#include <limits>

#include "../../Core/Utilities.h"
#include "../../Types/SimState.h"

ShovelId Shovel::GetBestShovel(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    ShovelId bestShovel;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();
    
    for (const Shovel& shovel : sim.shovels)
    {
        Position shovelPos = shovel.GetPosition();
        const float travelTime = Utilities::GetTravelTime(truckPos, shovelPos, truckSpeed);

        float queueTime = 0;

        // Trucks in queue
        const auto& queue = shovel.GetQueue();                                                              
        for (int i = 0; i < queue.size(); i++)  
        {                                                                                                   
            const Truck& qt = sim.trucks[queue[i].value];                                                   
            if (i == 0)                                                                                     
                queueTime += qt.EstTaskTimeRemaining(sim.currentTime); // remaining load, not full          
            else                                                                                            
                queueTime += shovel.TimeToLoad(qt);
        }

        // En route trucks closer than this truck
        for (const auto& t : sim.trucks)
        {
            if (t.GetId() == truck.GetId()) continue;
            const float tTravelTime = Utilities::GetTravelTime(t.GetPosition(), shovelPos, t.GetSpeed());
            if (t.GetState() == TruckState::Travelling
                && t.GetNextEvent().shovel == shovel.GetShovelId()
                && tTravelTime < travelTime)
            {
                queueTime += tTravelTime + shovel.TimeToLoad(t);
            }
        }
        
        const float score = (travelTime * travelTimePriority) + (queueTime * queuePriority);
        if (score < bestScore)
        {
            bestScore = score;
            bestShovel = shovel.GetShovelId();
        }
    }
    return bestShovel;
}
