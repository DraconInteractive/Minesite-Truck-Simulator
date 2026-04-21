#pragma once

#include <string>
#include <vector>
#include "../Types/SimState.h"
#include "../Types/DebugConfig.h"
#include "../Types/RoutingConfig.h"

struct TruckConfig   { int id; float ladenSpeed; float unladenSpeed; int capacity; };
struct ShovelConfig  { int id; float x, y; float loadSpeed; };
struct DumpConfig    { int id; float x, y; float dumpSpeed; };
struct SeedEvent     { int truckId; int shovelId; float arrivalTime; };



struct Config
{
    std::vector<TruckConfig>  trucks;
    std::vector<ShovelConfig> shovels;
    std::vector<DumpConfig>   dumps;
    std::vector<SeedEvent>    seedEvents;
    RoutingConfig             routing;
    DebugConfig               debug;

    static Config Load(const std::string& path);
    static SimState BuildSimState(const Config& cfg);
};
