#include "StationaryEntity.h"

#include <iomanip>
#include <limits>

#include "../../Core/Utilities.h"
#include "../../Navigation/Pathfinding.h"
#include "../../Types/SimState.h"

template<typename TSite>
int StationaryEntity::GetBestSite(const SimState& sim, const std::vector<TSite>& sites, const Truck& truck, float travelTimePriority, float queuePriority)
{
    int bestSite = -1;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();

    std::cout << "\nAssessing best site\n";
    std::cout << "Site Count: " << static_cast<int>(sites.size()) << "\n";

    auto oldFlags = std::cout.flags();
    std::cout << std::fixed << std::setprecision(2);
    
    for (const StationaryEntity& site : sites)
    {
        Position shovelPos = site.GetPosition();
        int siteId = site.GetId();
        
        const float travelTime = Navigation::GetTravelTimeByPosition(sim, truckPos, shovelPos, truckSpeed);
        std::cout << "Direct travel time: " << travelTime << "\n";
        
        float queueTime = 0;

        // Trucks in queue
        const auto& queue = site.GetQueue();
        std::cout << "Trucks in queue: " << static_cast<int>(queue.size()) << "\n";
        for (int i = 0; i < queue.size(); i++)  
        {                                                                                                   
            const Truck& qt = sim.trucks[queue[i].value];                                                   
            if (i == 0)                                                                                     
                queueTime += qt.EstTaskTimeRemaining(sim.currentTime); // remaining load, not full          
            else                                                                                            
                queueTime += site.TimeToProcess(qt);
        }

        int enRoute = 0;
        // En route trucks closer than this truck
        for (const auto& t : sim.trucks)
        {
            if (t.GetId() == truck.GetId()) continue;
            if (t.GetState() != TruckState::Travelling) continue;
            
            const float tTravelTime = t.EstTaskTimeRemaining(sim.currentTime);
            const auto& nextEvt = t.GetNextEvent();
            if (site.IsDestination(nextEvt) && tTravelTime < travelTime)
            {
                queueTime += tTravelTime + site.TimeToProcess(t);
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
            bestSite = siteId;
        }
    }

    std::cout << "Best score: " << bestScore << "\n ------- \n\n";
    std::cout.flags(oldFlags);

    return bestSite;
}

template int StationaryEntity::GetBestSite<Shovel>(const
  SimState&, const std::vector<Shovel>&, const Truck&, float,     
  float);

template int StationaryEntity::GetBestSite<Dump>(const
SimState&, const std::vector<Dump>&, const Truck&, float,       
float); 

