#include <iostream>
#include <string>
#include <vector>

#include "Core/console.h"
#include "Entities/Mobile/Truck.h"
#include "Entities/Stationary/Dump.h"
#include "Entities/Stationary/Shovel.h"

struct AppState
{
    bool running;
};

// Create structs for entity id classification - stops me from assigning shovel ID to truck ID function
constexpr int INVALID_ENTITY_ID = -1;

struct TruckId
{
    int value = INVALID_ENTITY_ID;
};

struct ShovelId
{
    int value = INVALID_ENTITY_ID;
};

struct DumpId
{
    int value = INVALID_ENTITY_ID;
};

enum class EventType : uint8_t
{
    TruckArriveShovel,
    TruckFinishLoading,
    TruckArriveDump,
    TruckFinishDumping
};

struct Event
{
    double time;
    TruckId truck;
    ShovelId shovel;
    DumpId dump;
    EventType type;
};

int main(int argc, char* argv[])
{
    AppState state;
    state.running = true;
    
    console::enableAnsi();
    // console::enableRawInput(); // For later - better navigation etc

    std::vector<Truck> trucks;
    std::vector<Shovel> shovels;
    std::vector<Dump> dumps;

    // TODO move the above structs to their own area, and guide this init from a config file

    trucks.reserve(3);
    for (int i = 0; i < 3; i++)
    {
        // Use emplace to construct in-place instead of construct->copy
        trucks.emplace_back(i, 60, 100, 0);
    }

    shovels.reserve(1);
    shovels = { Shovel(0, 10) };

    dumps.reserve(1);
    dumps = { Dump(0)};
    
    while (state.running)
    {
        std::string inputLine;
        std::getline(std::cin, inputLine);
        std::cout << "Echo: " + inputLine;
    }
    return 0;
}
