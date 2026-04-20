#include "Shovel.h"
#include <limits>

#include "../../Core/Utilities.h"

ShovelId Shovel::GetBestShovel(Truck& truck, std::vector<Shovel>& shovels, float travelTimePriority, float queuePriority)
{
    ShovelId bestShovel;
    float bestScore = std::numeric_limits<float>::max();

    const Position truckPos = truck.GetPosition();
    const float truckSpeed = truck.GetSpeed();
    
    for (Shovel& shovel : shovels)
    {
        const float travelTime = Utilities::GetTravelTime(truckPos, shovel.GetPosition(), truckSpeed) * travelTimePriority;
        const float queueWait = static_cast<float>(shovel.TrucksInQueue()) * shovel.TimeToLoad(truck) * queuePriority;
        const float score = travelTime + queueWait;
        if (score < bestScore)
        {
            bestScore = score;
            bestShovel = shovel.GetShovelId();
        }
    }
    return bestShovel;
}
