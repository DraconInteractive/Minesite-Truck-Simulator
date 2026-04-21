#include "Dump.h"
#include "../../Core/Utilities.h"
#include <limits>

#include "../../Types/SimState.h"

DumpId Dump::GetBestDump(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    DumpId bestDump;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();
    
    for (const Dump& dump : sim.dumps)
    {
        Position dumpPos = dump.GetPosition();
        const float travelTime = Utilities::GetTravelTime(truckPos, dumpPos, truckSpeed);

        float queueTime = 0;

        // Trucks in queue
        const auto& queue = dump.GetQueue();                                                              
        for (int i = 0; i < queue.size(); i++)  
        {                                                                                                   
            const Truck& qt = sim.trucks[queue[i].value];                                                   
            if (i == 0)                                                                                     
                queueTime += qt.EstTaskTimeRemaining(sim.currentTime); // remaining load, not full          
            else                                                                                            
                queueTime += dump.TimeToDump(qt);
        }

        // En route trucks closer than this truck
        for (const auto& t : sim.trucks)
        {
            if (t.GetId() == truck.GetId()) continue;
            const float tTravelTime = Utilities::GetTravelTime(t.GetPosition(), dumpPos, t.GetSpeed());
            if (t.GetState() == TruckState::Travelling
                && t.GetNextEvent().dump == dump.GetDumpId()
                && tTravelTime < travelTime)
            {
                queueTime += tTravelTime + dump.TimeToDump(t);
            }
        }
        
        const float score = (travelTime * travelTimePriority) + (queueTime * queuePriority);
        if (score < bestScore)
        {
            bestScore = score;
            bestDump = dump.GetDumpId();
        }
    }
    return bestDump;
}
