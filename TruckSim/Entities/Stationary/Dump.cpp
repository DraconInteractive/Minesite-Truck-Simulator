#include "Dump.h"

#include <iomanip>

#include "../../Core/Utilities.h"
#include <limits>

#include "../../Types/SimState.h"

DumpId Dump::GetBestDump(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    DumpId bestDump;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();

    std::cout << "\nAssessing best dump\n";
    std::cout << "Dump Count: " << static_cast<int>(sim.dumps.size()) << "\n";

    auto oldFlags = std::cout.flags();
    std::cout << std::fixed << std::setprecision(2);
    
    for (const Dump& dump : sim.dumps)
    {
        Position dumpPos = dump.GetPosition();
        const float travelTime = Utilities::GetTravelTime(truckPos, dumpPos, truckSpeed);
        std::cout << "Direct travel time: " << travelTime << "\n";

        float queueTime = 0;

        // Trucks in queue
        const auto& queue = dump.GetQueue();
        std::cout << "Trucks in queue: " << static_cast<int>(queue.size()) << "\n";

        for (int i = 0; i < queue.size(); i++)  
        {                                                                                                   
            const Truck& qt = sim.trucks[queue[i].value];                                                   
            if (i == 0)                                                                                     
                queueTime += qt.EstTaskTimeRemaining(sim.currentTime); // remaining load, not full          
            else                                                                                            
                queueTime += dump.TimeToDump(qt);
        }

        int enRoute = 0;
        // En route trucks closer than this truck
        for (const auto& t : sim.trucks)
        {
            if (t.GetId() == truck.GetId()) continue;
            if (t.GetState() != TruckState::Travelling) continue;

            // const float tTravelTime = Utilities::GetTravelTime(t.GetPosition(), dumpPos, t.GetSpeed());
            const float tTravelTime = t.EstTaskTimeRemaining(sim.currentTime);
            if (t.GetNextEvent().dump == dump.GetDumpId() && tTravelTime < travelTime)
            {
                queueTime += tTravelTime + dump.TimeToDump(t);
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
            bestDump = dump.GetDumpId();
        }
    }

    std::cout << "Best score: " << bestScore << "\n ------- \n\n";
    std::cout.flags(oldFlags);
    
    return bestDump;
}
