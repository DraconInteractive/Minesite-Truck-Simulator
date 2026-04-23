#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "../Types/SimState.h"
#include "../Types/DebugConfig.h"
#include "../Types/RoutingConfig.h"

struct TruckConfig   { int id; float ladenSpeed; float unladenSpeed; int capacity; std::string loadoutFile; };
struct ShovelConfig  { int id; float x, y; float loadSpeed; };
struct DumpConfig    { int id; float x, y; float dumpSpeed; };
struct SeedEvent     { int truckId; float arrivalTime; };

struct PartConfig
{
    std::string name;
    float repairTime;
    float baseFailRate;
    float wearPerCycle;
};

struct LoadoutConfig
{
    std::string name;
    std::vector<PartConfig> parts;
};

struct Config
{
    std::vector<TruckConfig>  trucks;
    std::vector<ShovelConfig> shovels;
    std::vector<DumpConfig>   dumps;
    std::vector<SeedEvent>    seedEvents;
    std::unordered_map<std::string, LoadoutConfig> loadouts;
    
    RoutingConfig             routing;
    DebugConfig               debug;

    static Config Load(const std::string& path);
    static SimState BuildSimState(const Config& cfg);
};
