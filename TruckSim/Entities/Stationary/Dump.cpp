#include "Dump.h"
#include "../../Core/Utilities.h"
#include <limits>

DumpId Dump::GetBestDump(Truck& truck, std::vector<Dump>& dumps, float travelTimePriority, float queuePriority)
{
    DumpId bestDump;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();
    
    for (Dump& dump : dumps)
    {
        const float travelTime = Utilities::GetTravelTime(truckPos, dump.GetPosition(), truckSpeed) * travelTimePriority;
        const float queueWait = static_cast<float>(dump.TrucksInQueue()) * dump.TimeToDump(truck) * queuePriority;
        const float score = travelTime + queueWait;
        if (score < bestScore)
        {
            bestScore = score;
            bestDump = dump.GetDumpId();
        }
    }
    return bestDump;
}