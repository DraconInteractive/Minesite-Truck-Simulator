#include "Shovel.h"

#include <iomanip>
#include <limits>

#include "../../Core/Utilities.h"
#include "../../Types/SimState.h"

ShovelId Shovel::GetBestShovel(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    ShovelId bestShovel;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();

    std::cout << "\nAssessing best shovel\n";
    std::cout << "Shovel Count: " << static_cast<int>(sim.shovels.size()) << "\n";

    auto oldFlags = std::cout.flags();
    std::cout << std::fixed << std::setprecision(2);
    
    for (const Shovel& shovel : sim.shovels)
    {
        Position shovelPos = shovel.GetPosition();
        const float travelTime = Utilities::GetTravelTime(truckPos, shovelPos, truckSpeed);
        std::cout << "Direct travel time: " << travelTime << "\n";
        
        float queueTime = 0;

        // Trucks in queue
        const auto& queue = shovel.GetQueue();
        std::cout << "Trucks in queue: " << static_cast<int>(queue.size()) << "\n";
        for (int i = 0; i < queue.size(); i++)  
        {                                                                                                   
            const Truck& qt = sim.trucks[queue[i].value];                                                   
            if (i == 0)                                                                                     
                queueTime += qt.EstTaskTimeRemaining(sim.currentTime); // remaining load, not full          
            else                                                                                            
                queueTime += shovel.TimeToLoad(qt);
        }

        int enRoute = 0;
        // En route trucks closer than this truck
        for (const auto& t : sim.trucks)
        {
            if (t.GetId() == truck.GetId()) continue;
            if (t.GetState() != TruckState::Travelling) continue;
            
            //const float tTravelTime = Utilities::GetTravelTime(t.GetPosition(), shovelPos, t.GetSpeed());
            const float tTravelTime = t.EstTaskTimeRemaining(sim.currentTime);
            if (t.GetNextEvent().shovel == shovel.GetShovelId() && tTravelTime < travelTime)
            {
                queueTime += tTravelTime + shovel.TimeToLoad(t);
            }
            enRoute++;
        }

        std::cout << "Trucks en route: " << enRoute << "\n";
        std::cout << "Total queue time: " << queueTime << "\n";
        
        const float score = (travelTime * travelTimePriority) + (queueTime * queuePriority);
        std::cout << "Score: " << score << "\n\n";

        if (score < bestScore)
        {
            bestScore = score;
            bestShovel = shovel.GetShovelId();
        }
    }

    std::cout << "Best score: " << bestScore << "\n ------- \n\n";
    std::cout.flags(oldFlags);

    return bestShovel;
}
