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

int main(int argc, char* argv[])
{
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

    if (cfg.debug.logEvents)
        std::cout << "Sim Creation: Done\n";
    
    // Init rendering
    InitWindow(800,600, "TruckSim");
    Font font = LoadFontEx("C:/Windows/Fonts/arial.ttf", 20, 0, 0);

    if (cfg.debug.logEvents)
        std::cout << "Rendering: Done\n";

    while (!sim.evtQueue.empty() && sim.currentTime < timeCap)
    {
        Event evt = sim.evtQueue.top();
        sim.evtQueue.pop();

        sim.currentTime = evt.time;

        if (cfg.debug.logEvents)
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
