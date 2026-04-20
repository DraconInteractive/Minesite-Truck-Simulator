#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "Core/Event.h"
#include "Core/console.h"

#include <raylib.h>

#include "Core/Simulation.h"
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

    sim.trucks[0].StartTask(0, t0Evt);
    sim.trucks[1].StartTask(0, t1Evt);
    sim.trucks[2].StartTask(0, t2Evt);
    sim.trucks[3].StartTask(0, t3Evt);
    sim.trucks[4].StartTask(0, t4Evt);
    
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
                Simulation::HandleTruckArriveShovel(sim, evt);
                break;
            }
            case EventType::TruckFinishLoading:
            {
                Simulation::HandleTruckFinishLoading(sim, evt);
                break;
            }
            case EventType::TruckArriveDump:
            {
                Simulation::HandleTruckArriveDump(sim, evt);
                break;
            }
            case EventType::TruckFinishDumping:
            {
                Simulation::HandleTruckFinishDumping(sim, evt);
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
