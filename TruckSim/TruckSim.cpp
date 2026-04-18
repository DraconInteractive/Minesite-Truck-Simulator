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
    
    trucks.reserve(5);
    for (int i = 0; i < 5; i++)
    {
        // We're running in tonnes for now, saying Cat 797 at 400 tonnes.
        // Speed = 30kmh, but TODO make it slower loader (15-25) than empty (30-45)
        // Use emplace to construct in-place instead of construct->copy
        trucks.emplace_back(i, 30, 400, 0);
    }

    // Running with a load speed of 120 tonnes per minute
    // Positions need to be in km I think, to be converted to km/m to fit in with minute steps
    shovels.reserve(2);
    shovels = {
        Shovel(0, Position{5, 10}, 120),
        Shovel(1, Position{12, 5}, 120)
    };

    // Running with dump speed of 160 tonnes per minute
    dumps.reserve(3);
    dumps = {
        Dump(0, Position{-3, 6}, 160),
        Dump(1, Position{5, -3}, 160),
        Dump(2, Position{-5, 3}, 160)
    };

    // Seed events. Manually set truck targets for first phase. Stagger entry for fun
    evtQueue.push(Event{2, TruckId{0}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{3, TruckId{1}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{4, TruckId{2}, ShovelId{0}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{5, TruckId{3}, ShovelId{1}, {}, EventType::TruckArriveShovel});
    evtQueue.push(Event{5, TruckId{4}, ShovelId{1}, {}, EventType::TruckArriveShovel});

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
                    evtQueue.push(Event{currentTime + shovel.TimeToLoad(trucks[evt.truck.value]), evt.truck, evt.shovel, {}, EventType::TruckFinishLoading});
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

                truckLeavingShovel.Fill();
                    
                shovel.DequeueTruck(); // Truck has finished loading, and is leaving, so remove from queue

                // Move truck to dump
                truckLeavingShovel.SetState(TruckState::Travelling);
                DumpId bestDump = Dump::GetBestDump(truckLeavingShovel, dumps);
                const double travelTimeToDump = Utilities::GetTravelTime(shovel.GetPosition(), dumps[bestDump.value].GetPosition(), truckLeavingShovel.GetSpeed());
                truckLeavingShovel.targetPosition = dumps[bestDump.value].GetPosition();
                evtQueue.push(Event{currentTime + travelTimeToDump, evt.truck, {}, bestDump, EventType::TruckArriveDump});

                // Process next truck in queue
                if (shovel.TrucksInQueue() > 0)
                {
                    TruckId nextTruck = shovel.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Loading);
                    evtQueue.push(Event{currentTime + shovel.TimeToLoad(trucks[nextTruck.value]), nextTruck, evt.shovel, {}, EventType::TruckFinishLoading});
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
                    evtQueue.push(Event{currentTime + dump.TimeToDump(trucks[evt.truck.value]), evt.truck, {}, evt.dump, EventType::TruckFinishDumping});
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

                truckLeavingDump.Empty();
                    
                dump.DequeueTruck();
                    
                truckLeavingDump.SetState(TruckState::Travelling);
                ShovelId bestShovel = Shovel::GetBestShovel(truckLeavingDump, shovels);
                const double travelTimeToShovel = Utilities::GetTravelTime(dump.GetPosition(), shovels[bestShovel.value].GetPosition(), truckLeavingDump.GetSpeed());
                truckLeavingDump.targetPosition = shovels[bestShovel.value].GetPosition();
                    
                evtQueue.push(Event{currentTime + travelTimeToShovel, evt.truck, bestShovel, {}, EventType::TruckArriveShovel});
                    
                // Start processing next truck in queue
                if (dump.TrucksInQueue() > 0)
                {
                    TruckId nextTruck = dump.GetFirst();
                    trucks[nextTruck.value].SetState(TruckState::Dumping);
                    evtQueue.push(Event{currentTime + dump.TimeToDump(trucks[nextTruck.value]), nextTruck, {}, evt.dump, EventType::TruckFinishDumping});
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
