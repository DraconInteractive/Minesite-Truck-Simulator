#include "Shovel.h"
#include <limits>

#include "../../Core/Utilities.h"

ShovelId Shovel::GetBestShovel(Truck& truck, std::vector<Shovel>& shovels, float distPriority, float queuePriority)
{
    ShovelId bestShovel;
    double bestScore = std::numeric_limits<double>::max();

    Position truckPos = truck.GetPosition();
    float truckSpeed = truck.GetSpeed();
    
    for (Shovel& shovel : shovels)
    {
        const double dist = Utilities::GetTravelTime(truckPos, shovel.GetPosition(), truckSpeed) * distPriority;
        const double queueWait = shovel.TrucksInQueue() * shovel.TimeToLoad(truck) * queuePriority;
        const double score = dist + queueWait;
        if (score < bestScore)
        {
            bestScore = score;
            bestShovel = shovel.GetShovelId();
        }
    }
    return bestShovel;
}
