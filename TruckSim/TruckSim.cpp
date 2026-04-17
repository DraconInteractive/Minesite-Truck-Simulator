#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "Core/Event.h"
#include "Core/console.h"
#include "Core/Utilities.h"

#include <raylib.h>
#include "Rendering/Renderer.h"

#include "Entities/Mobile/Truck.h"
#include "Entities/Stationary/Dump.h"
#include "Entities/Stationary/Shovel.h"

struct AppState
{
    bool running;
};

struct CompareByTime {                                       
    bool operator()(const Event& a, const Event& b) {        
        return a.time > b.time;  // > makes it a min-heap  
    }                                                        
};

int main(int argc, char* argv[])
{
    std::cout << "Start\n";
    AppState state;
    state.running = true;
    
    console::enableAnsi();
    // console::enableRawInput(); // For later - better navigation etc

    std::cout << "ANSI enabled\n";

    std::vector<Truck> trucks;
    std::vector<Shovel> shovels;
    std::vector<Dump> dumps;

    std::priority_queue<Event, std::vector<Event>, CompareByTime> evtQueue; // smallest event time at top
    double currentTime = 0;
    
    trucks.reserve(3);
    for (int i = 0; i < 3; i++)
    {
        // Use emplace to construct in-place instead of construct->copy
        trucks.emplace_back(i, 60, 100, 0);
    }

    shovels.reserve(2);
    shovels = {
        Shovel(0, Position{5, 10}, 10),
        Shovel(1, Position{12, 5}, 10)
    };
    
    dumps.reserve(3);
    dumps = {
        Dump(0, Position{-3, 6}, 5),
        Dump(1, Position{5, -3}, 5),
        Dump(2, Position{-5, 3}, 5)
    };

    evtQueue.push(Event{0, TruckId{0}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{1, TruckId{1}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{5, TruckId{2}, ShovelId{0}, {}, EventType::TruckArriveShovel});

    std::cout << "Queue Setup\n";

    double timeCap = 1000;

    // Init rendering
    InitWindow(800,600, "TruckSim");
    Font font = LoadFontEx("C:/Windows/Fonts/arial.ttf", 20, 0, 0);

    std::cout << "Rendering setup\n";

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
                Shovel& shovel = shovels[evt.shovel.value];
                Truck& truck = trucks[evt.truck.value];
                truck.SetPosition(shovel.GetPosition());
                    
                shovel.EnqueueTruck(evt.truck);

                if (shovel.TrucksInQueue() == 1) // Only queue event if first in line, trucks in line will trigger when first leaves queue
                {
                    trucks[evt.truck.value].SetState(TruckState::Loading);
                    evtQueue.push(Event{currentTime + shovel.TimeToLoad(), evt.truck, evt.shovel, {}, EventType::TruckFinishLoading});
                }
                else
                {
                    trucks[evt.truck.value].SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishLoading:
            {
                Shovel& shovel = shovels[evt.shovel.value];
                Truck& truckLeavingShovel = trucks[evt.truck.value];

                shovel.DequeueTruck(); // Truck has finished loading, and is leaving, so remove from queue
            
                truckLeavingShovel.SetState(TruckState::Travelling);
                DumpId bestDump = Dump::GetBestDump(truckLeavingShovel, dumps);
                const double travelTimeToDump = Utilities::GetTravelTime(shovel.GetPosition(), dumps[bestDump.value].GetPosition(), truckLeavingShovel.GetSpeed());
                evtQueue.push(Event{currentTime + travelTimeToDump, evt.truck, {}, bestDump, EventType::TruckArriveDump});
            
                if (shovel.TrucksInQueue() > 0)
                {
                    // Start loading next truck
                    TruckId nextTruck = shovel.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Loading);
                    evtQueue.push(Event{currentTime + shovel.TimeToLoad(), nextTruck, evt.shovel, {}, EventType::TruckFinishLoading});
                }
            
                break;
            }
            case EventType::TruckArriveDump:
            {
                Dump& dump = dumps[evt.dump.value];
                Truck& truck = trucks[evt.truck.value];

                truck.SetPosition(dump.GetPosition());
                dump.EnqueueTruck(evt.truck);

                if (dump.TrucksInQueue() == 1)
                {
                    trucks[evt.truck.value].SetState(TruckState::Dumping);
                    evtQueue.push(Event{currentTime + dump.TimeToDump(), evt.truck, {}, evt.dump, EventType::TruckFinishDumping});
                }
                else
                {
                    trucks[evt.truck.value].SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishDumping:
            {
                Dump& dump = dumps[evt.dump.value];
                Truck& truckLeavingDump = trucks[evt.truck.value];
                    
                dump.DequeueTruck();

                // here we could navigate back to the shovel, but to avoid a constant loop we're going to say the truck is resting after a single trip

                truckLeavingDump.SetState(TruckState::Travelling);
                ShovelId bestShovel = Shovel::GetBestShovel(truckLeavingDump, shovels);
                const double travelTimeToShovel = Utilities::GetTravelTime(dump.GetPosition(), shovels[bestShovel.value].GetPosition(), truckLeavingDump.GetSpeed());
                evtQueue.push(Event{currentTime + travelTimeToShovel, evt.truck, bestShovel, {}, EventType::TruckArriveShovel});
                    
                // Start processing next truck in queue
                if (dump.TrucksInQueue() > 0)
                {
                    TruckId nextTruck = dump.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Dumping);
                    evtQueue.push(Event{currentTime + dump.TimeToDump(), nextTruck, {}, evt.dump, EventType::TruckFinishDumping});
                }
                break;
            }
        }
        
        if (!WindowShouldClose())
        {
            Render(shovels, dumps, trucks, evt, currentTime, font);
            WaitForKeypress();
        }
        else
        {
            break;
        }
    }
    
    CloseWindow();
    return 0;
}
