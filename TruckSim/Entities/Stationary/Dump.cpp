#include "Dump.h"

#include <iomanip>

#include "../../Core/Utilities.h"
#include <limits>

#include "../../Types/SimState.h"

DumpId Dump::GetBestDump(const SimState& sim, const Truck& truck, float travelTimePriority, float queuePriority)
{
    int siteId = StationaryEntity::GetBestSite(sim, sim.dumps, truck, travelTimePriority, queuePriority);
    return DumpId{siteId};
}
