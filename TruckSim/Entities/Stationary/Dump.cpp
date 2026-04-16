#include "Dump.h"
#include "../../Core/Utilities.h"
#include <limits>

DumpId Dump::GetBestDump(Truck& truck, std::vector<Dump>& dumps, float distPriority, float queuePriority)
{
    DumpId bestDump;
    double bestScore = std::numeric_limits<double>::max();

    Position truckPos = truck.GetPosition();
    float truckSpeed = truck.GetSpeed();
    
    for (Dump& dump : dumps)
    {
        const double dist = Utilities::GetTravelTime(truckPos, dump.GetPosition(), truckSpeed) * distPriority;
        const double queueWait = dump.TrucksInQueue() * dump.TimeToDump() * queuePriority;
        const double score = dist + queueWait;
        if (score < bestScore)
        {
            bestScore = score;
            bestDump = dump.GetDumpId();
        }
    }
    return bestDump;
}