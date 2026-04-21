#include "Shovel.h"

#include <iomanip>
#include <limits>

#include "../../Core/Utilities.h"
#include "../../Types/SimState.h"

ShovelId Shovel::GetBestShovel(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    int siteId = StationaryEntity::GetBestSite(sim, sim.shovels, truck, travelTimePriority, queuePriority);
    return ShovelId{siteId};
}