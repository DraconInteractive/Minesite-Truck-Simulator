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

#include "Types/SimState.h"

int main(int argc, char* argv[])
{
    std::cout << "Start\n";

    SimState sim;
    
    console::enableAnsi();
    // console::enableRawInput(); // For later - better navigation etc

    std::cout << "ANSI enabled\n";
    
    sim.trucks.reserve(5);
    for (int i = 0; i < 5; i++)
    {
        // We're running in tonnes for now, saying Cat 797 at 400 tonnes.
        // Speed = 30kmh, but TODO make it slower loader (15-25) than empty (30-45)
        // Use emplace to construct in-place instead of construct->copy
        sim.trucks.emplace_back(i, 30, 400, 0);
    }

    // Running with a load speed of 120 tonnes per minute
    // Positions need to be in km I think, to be converted to km/m to fit in with minute steps
    sim.shovels.reserve(2);
    sim.shovels = {
        Shovel(0, Position{5, 10}, 120),
        Shovel(1, Position{12, 5}, 120)
    };

    // Running with dump speed of 160 tonnes per minute
    sim.dumps.reserve(3);
    sim.dumps = {
        Dump(0, Position{-3, 6}, 160),
        Dump(1, Position{5, -3}, 160),
        Dump(2, Position{-5, 3}, 160)
    };

    // Seed events. Manually set truck targets for first phase. Stagger entry for fun
    // Setting these manually for now, no loop. TODO: Introduce config loading
    Event t0Evt = Event{2, TruckId{0}, ShovelId{0}, {}, EventType::TruckArriveShovel};
    Event t1Evt = Event{3, TruckId{1}, ShovelId{0}, {}, EventType::TruckArriveShovel};
    Event t2Evt = Event{4, TruckId{2}, ShovelId{0}, {}, EventType::TruckArriveShovel};
    Event t3Evt = Event{5, TruckId{3}, ShovelId{1}, {}, EventType::TruckArriveShovel};
    Event t4Evt = Event{5, TruckId{4}, ShovelId{1}, {}, EventType::TruckArriveShovel};

    sim.trucks[0].StartTask(2, t0Evt);
    sim.trucks[1].StartTask(3, t1Evt);
    sim.trucks[2].StartTask(4, t2Evt);
    sim.trucks[3].StartTask(5, t3Evt);
    sim.trucks[4].StartTask(5, t4Evt);
    
    sim.evtQueue.push(t0Evt);
    sim.evtQueue.push(t1Evt);
    sim.evtQueue.push(t2Evt);
    sim.evtQueue.push(t3Evt);
    sim.evtQueue.push(t4Evt);
    
    std::cout << "Queue Setup\n";

    float timeCap = 1000;

    // Init rendering
    InitWindow(800,600, "TruckSim");
    Font font = LoadFontEx("C:/Windows/Fonts/arial.ttf", 20, 0, 0);

    std::cout << "Rendering setup\n";

    while (!sim.evtQueue.empty() && sim.currentTime < timeCap)
    {
        Event evt = sim.evtQueue.top();
        sim.evtQueue.pop();

        sim.currentTime = evt.time;

        std::cout << "Processing event [" << EventTypeToString(evt.type) << "] at time: " << sim.currentTime << "\n";

        switch (evt.type)
        {
            case EventType::TruckArriveShovel:
            {
                Shovel& shovel = sim.shovels[evt.shovel.value];
                Truck& truck = sim.trucks[evt.truck.value];
                truck.SetPosition(shovel.GetPosition());
                    
                shovel.EnqueueTruck(evt.truck);

                if (shovel.TrucksInQueue() == 1) // Only queue event if first in line, trucks in line will trigger when first leaves queue
                {
                    float duration = shovel.TimeToLoad(sim.trucks[evt.truck.value]) ;
                    auto finishLoadingEvt = Event{sim.currentTime + duration, evt.truck, evt.shovel, {}, EventType::TruckFinishLoading};
                    truck.StartTask(sim.currentTime, finishLoadingEvt);
                    sim.trucks[evt.truck.value].SetState(TruckState::Loading);
                    sim.evtQueue.push(finishLoadingEvt);
                }
                else
                {
                    sim.trucks[evt.truck.value].SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishLoading:
            {
                Shovel& shovel = sim.shovels[evt.shovel.value];
                Truck& truckLeavingShovel = sim.trucks[evt.truck.value];

                truckLeavingShovel.Fill();
                    
                shovel.DequeueTruck(); // Truck has finished loading, and is leaving, so remove from queue

                // Move truck to dump
                truckLeavingShovel.SetState(TruckState::Travelling);
                DumpId bestDump = Dump::GetBestDump(truckLeavingShovel, sim.dumps);
                const float travelTimeToDump = Utilities::GetTravelTime(shovel.GetPosition(), sim.dumps[bestDump.value].GetPosition(), truckLeavingShovel.GetSpeed());
                truckLeavingShovel.targetPosition = sim.dumps[bestDump.value].GetPosition();
                auto arriveAtDumpEvt = Event{sim.currentTime + travelTimeToDump, evt.truck, {}, bestDump, EventType::TruckArriveDump};
                truckLeavingShovel.StartTask(sim.currentTime, arriveAtDumpEvt);
                sim.evtQueue.push(arriveAtDumpEvt);

                // Process next truck in queue
                if (shovel.TrucksInQueue() > 0)
                {
                    const TruckId nextTruckId = shovel.GetFirst();
                    Truck& nextTruck = sim.trucks[nextTruckId.value];
                    nextTruck.SetState(TruckState::Loading);
                    float loadDuration = shovel.TimeToLoad(nextTruck);
                    auto finishLoadingEvt = Event{sim.currentTime + loadDuration, nextTruckId, evt.shovel, {}, EventType::TruckFinishLoading};
                    nextTruck.StartTask(sim.currentTime, finishLoadingEvt);
                    sim.evtQueue.push(finishLoadingEvt);
                }
                break;
            }
            case EventType::TruckArriveDump:
            {
                Dump& dump = sim.dumps[evt.dump.value];
                Truck& truck = sim.trucks[evt.truck.value];

                truck.SetPosition(dump.GetPosition());
                dump.EnqueueTruck(evt.truck);

                if (dump.TrucksInQueue() == 1)
                {
                    truck.SetState(TruckState::Dumping);
                    float duration = dump.TimeToDump(truck);
                    auto finishDumpingEvt = Event{sim.currentTime + duration, evt.truck, {}, evt.dump, EventType::TruckFinishDumping};
                    truck.StartTask(sim.currentTime, finishDumpingEvt);
                    sim.evtQueue.push(finishDumpingEvt);
                }
                else
                {
                    truck.SetState(TruckState::Queueing);
                }
                break;
            }
            case EventType::TruckFinishDumping:
            {
                Dump& dump = sim.dumps[evt.dump.value];
                Truck& truckLeavingDump = sim.trucks[evt.truck.value];

                truckLeavingDump.Empty();
                    
                dump.DequeueTruck();
                    
                truckLeavingDump.SetState(TruckState::Travelling);
                ShovelId bestShovel = Shovel::GetBestShovel(truckLeavingDump, sim.shovels);
                const float travelTimeToShovel = Utilities::GetTravelTime(dump.GetPosition(), sim.shovels[bestShovel.value].GetPosition(), truckLeavingDump.GetSpeed());
                truckLeavingDump.targetPosition = sim.shovels[bestShovel.value].GetPosition();
                auto arriveAtShovelEvt = Event{sim.currentTime + travelTimeToShovel, evt.truck, bestShovel, {}, EventType::TruckArriveShovel};
                truckLeavingDump.StartTask(sim.currentTime, arriveAtShovelEvt);
                sim.evtQueue.push(arriveAtShovelEvt);
                    
                // Start processing next truck in queue
                if (dump.TrucksInQueue() > 0)
                {
                    TruckId nextTruckId = dump.GetFirst();
                    Truck& nextTruck = sim.trucks[nextTruckId.value];
                    nextTruck.SetState(TruckState::Dumping);
                    float dumpDuration = dump.TimeToDump(nextTruck);
                    Event finishDumpEvtEvent{sim.currentTime + dumpDuration, nextTruckId, {}, evt.dump, EventType::TruckFinishDumping};
                    nextTruck.StartTask(sim.currentTime, finishDumpEvtEvent);
                    sim.evtQueue.push(finishDumpEvtEvent);
                }
                break;
            }
        }
        
        if (!WindowShouldClose())
        {
            Render(sim, evt, font);
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
