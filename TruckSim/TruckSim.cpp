#include <iostream>
#include <queue>
#include <string>

#include "Core/Event.h"
#include "Core/console.h"

#include <raylib.h>

#include "Core/Config.h"
#include "Core/Simulation.h"
#include "Rendering/Renderer.h"

#include "Types/SimState.h"

static void DispatchEvent(SimState& sim, const Event& evt, bool logEvents)
{
    if (logEvents)
        std::cout << "Processing event [" << EventTypeToString(evt.type) << "] at time: " << sim.currentTime << "\n";

    switch (evt.type)
    {
    case EventType::TruckEnterSimulation:
        Simulation::HandleTruckEnterSimulation(sim, evt); break;
    case EventType::TruckArriveShovel:
        Simulation::HandleTruckArriveShovel(sim, evt);    break;
    case EventType::TruckFinishLoading:
        Simulation::HandleTruckFinishLoading(sim, evt);   break;
    case EventType::TruckArriveDump:
        Simulation::HandleTruckArriveDump(sim, evt);      break;
    case EventType::TruckFinishDumping:
        Simulation::HandleTruckFinishDumping(sim, evt);   break;
    case EventType::TruckPartFail:
        Simulation::HandleTruckPartFail(sim, evt);        break;
    case EventType::TruckPartFixed:
        Simulation::HandleTruckPartFixed(sim, evt);       break;
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Initializing...\n";
    std::cout << "Initializing...\n";
    
    Config cfg;
    
    try {                                                                                               
        cfg = Config::Load("sim.json");                                                                 
    } catch (const std::exception& e) {                                                                 
        std::cerr << "Failed to load config: " << e.what() << "\n";                                     
        return 1;                                                                                       
    }
    
    if (cfg.debug.logEvents)
        std::cout << "Start\n";
    
    console::enableAnsi();

    if (cfg.debug.logEvents)
        std::cout << "ANSI: Enabled\n";
    
    SimState sim = Config::BuildSimState(cfg);
    float timeCap = cfg.debug.timeCap;
    float simSpeed = 30.f;
    
    if (cfg.debug.logEvents)
        std::cout << "Sim Creation: Done\n";
    
    // Init rendering
    InitWindow(800,600, "TruckSim");
    SetTargetFPS(60);
    Font font = LoadFontEx("C:/Windows/Fonts/arial.ttf", 20, 0, 0);

    if (cfg.debug.logEvents)
        std::cout << "Rendering Init: Done\n";

    // start paused
    Event lastEvt = {};

    while (!WindowShouldClose())
    {
        if (!sim.isPaused && !sim.evtQueue.empty() && sim.currentTime < timeCap)
        {
            const float targetTime = sim.currentTime + simSpeed * GetFrameTime();
            while (!sim.evtQueue.empty() && sim.evtQueue.top().time <= targetTime)
            {
                lastEvt = sim.evtQueue.top();
                sim.evtQueue.pop();
                sim.currentTime = lastEvt.time;
                DispatchEvent(sim, lastEvt, cfg.debug.logEvents);
            }
            sim.currentTime = targetTime;
        }

        BeginDrawing();
        if (IsKeyPressed(KEY_SPACE))
        {
            sim.isPaused = !sim.isPaused;
        }
        Render(sim, lastEvt, font);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
