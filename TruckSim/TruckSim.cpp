#include <cmath>
#include <iostream>
#include <queue>
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

std::string EventTypeToString(EventType type)
{
    switch (type)
    {
        case EventType::TruckArriveShovel:      return "TruckArriveShovel";
        case EventType::TruckFinishLoading:     return "TruckFinishLoading";
        case EventType::TruckArriveDump:        return "TruckArriveDump";
        case EventType::TruckFinishDumping:     return "TruckFinishDumping";
    }
}

struct Event
{
    double time;
    TruckId truck;
    ShovelId shovel;
    DumpId dump;
    EventType type;
};

double travelTime(Position a, Position b, float speed)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    return distance / speed;
}

struct CompareByTime {                                       
    bool operator()(const Event& a, const Event& b) {        
        return a.time > b.time;  // > makes it a min-heap  
    }                                                        
};

int main(int argc, char* argv[])
{
    // TODO make MineState object
    // Tracks variables at a global scale
    // This will include items such as priorities for different materials etc
    // Then, feed this into event calls so that entities can factor that into decision making
    // This could be what material to load, or it could facilitate what shovel to target (to ensure lowest queue time)
    
    AppState state;
    state.running = true;
    
    console::enableAnsi();
    // console::enableRawInput(); // For later - better navigation etc

    std::vector<Truck> trucks;
    std::vector<Shovel> shovels;
    std::vector<Dump> dumps;

    std::priority_queue<Event, std::vector<Event>, CompareByTime> evtQueue; // smallest event time at top
    double currentTime = 0;
    
    // TODO move the above structs to their own area, and guide this init from a config file

    trucks.reserve(3);
    for (int i = 0; i < 3; i++)
    {
        // Use emplace to construct in-place instead of construct->copy
        trucks.emplace_back(i, 60, 100, 0);
    }

    shovels.reserve(1);
    shovels = { Shovel(0, Position{5, 10}, 10) };

    dumps.reserve(1);
    dumps = { Dump(0, Position{-3, 6}, 5)};

    evtQueue.push(Event{0, TruckId{0}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{1, TruckId{1}, ShovelId{0}, {}, EventType::TruckArriveShovel});

    double timeCap = 1000;
    
    while (!evtQueue.empty() && currentTime < timeCap)
    {
        Event evt = evtQueue.top();
        evtQueue.pop();

        currentTime = evt.time;

        std::cout << "Processing event [" << EventTypeToString(evt.type) << "] at time: " << currentTime << "\n";
        
        switch (evt.type)
        {
            case EventType::TruckArriveShovel:
            {
                Shovel& arriveShovel = shovels[evt.shovel.value];

                arriveShovel.EnqueueTruck(evt.truck);

                if (arriveShovel.TrucksInQueue() == 1) // Only queue event if first in line, trucks in line will trigger when first leaves queue
                {
                    trucks[evt.truck.value].SetState(TruckState::Loading);
                    evtQueue.push(Event{currentTime + arriveShovel.TimeToLoad(), evt.truck, evt.shovel, {}, EventType::TruckFinishLoading});
                }
                else
                {
                    trucks[evt.truck.value].SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishLoading:
            {
                Shovel& leaveShovel = shovels[evt.shovel.value];
                Truck& truckLeavingShovel = trucks[evt.truck.value];

                leaveShovel.DequeueTruck(); // Truck has finished loading, and is leaving, so remove from queue
            
                truckLeavingShovel.SetState(TruckState::Travelling);
                const double travelTimeToDump = travelTime(leaveShovel.GetPosition(), dumps[0].GetPosition(), truckLeavingShovel.GetSpeed());
                evtQueue.push(Event{currentTime + travelTimeToDump, evt.truck, {}, DumpId{0}, EventType::TruckArriveDump});
            
                if (leaveShovel.TrucksInQueue() > 0)
                {
                    // Start loading next truck
                    TruckId nextTruck = leaveShovel.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Loading);
                    evtQueue.push(Event{currentTime + leaveShovel.TimeToLoad(), nextTruck, evt.shovel, {}, EventType::TruckFinishLoading});
                }
            
                break;
            }
            case EventType::TruckArriveDump:
            {
                Dump& arriveDump = dumps[evt.dump.value];
                arriveDump.EnqueueTruck(evt.truck);

                if (arriveDump.TrucksInQueue() == 1)
                {
                    trucks[evt.truck.value].SetState(TruckState::Dumping);
                    evtQueue.push(Event{currentTime + arriveDump.TimeToDump(), evt.truck, {}, evt.dump, EventType::TruckFinishDumping});
                }
                else
                {
                    trucks[evt.truck.value].SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishDumping:
            {
                Dump& leaveDump = dumps[evt.dump.value];
                Truck& truckLeavingDump = trucks[evt.truck.value];
                    
                leaveDump.DequeueTruck();

                // here we could navigate back to the shovel, but to avoid a constant loop we're going to say the truck is resting after a single trip

                truckLeavingDump.SetState(TruckState::Travelling);
                const double travelTimeToShovel = travelTime(leaveDump.GetPosition(), shovels[0].GetPosition(), truckLeavingDump.GetSpeed());
                evtQueue.push(Event{currentTime + travelTimeToShovel, evt.truck, ShovelId{0}, {}, EventType::TruckArriveShovel});
                    
                // Start processing next truck in queue
                if (leaveDump.TrucksInQueue() > 0)
                {
                    TruckId nextTruck = leaveDump.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Dumping);
                    evtQueue.push(Event{currentTime + leaveDump.TimeToDump(), nextTruck, {}, evt.dump, EventType::TruckFinishDumping});
                }
                break;
            }
        }
    }
    while (state.running)
    {
        std::string inputLine;
        std::getline(std::cin, inputLine);
        std::cout << "Echo: " + inputLine;
    }
    return 0;
}
