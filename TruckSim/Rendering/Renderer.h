#pragma once

#include "raylib.h"
#include "../Types/Position.h"
#include "../Types/SimState.h"
#include "../Entities/Mobile/Truck.h"
#include "../Entities/Stationary/Shovel.h"
#include "../Entities/Stationary/Dump.h"

inline Vector2 worldToScreen(Position pos, float scale = 20.f, Vector2 offset = {400, 300})
{
    return { offset.x + pos.x * scale, offset.y - pos.y * scale };
}

inline void WaitForKeypress()
{
    while (!WindowShouldClose() && GetKeyPressed() == 0)
        PollInputEvents();
}

inline void RenderTruck(const Truck& t, Vector2 v, float labelX, float labelY, Font labelFont)
{
    Color c;
    switch (t.GetState())
    {
    case TruckState::Idle:       c = GRAY;    break;
    case TruckState::Travelling: c = SKYBLUE; break;
    case TruckState::Loading:    c = GREEN;   break;
    case TruckState::Queueing:   c = ORANGE;  break;
    case TruckState::Dumping:    c = PURPLE;  break;
    case TruckState::Broken:     c = RED;     break;
    default: c = WHITE; break;
    }
    
    DrawCircleV(v, 8, c);
    
    DrawTextEx(labelFont, ("T" + std::to_string(t.GetId())).c_str(), {labelX, labelY}, 16, 1, WHITE);
}

inline void Render(const SimState& sim, Event evt, Font font)
{
    ClearBackground(BLACK);

    // Origin crosshair
    DrawLine(0, 300, 800, 300, DARKGRAY);
    DrawLine(400, 0, 400, 600, DARKGRAY);

    // Shovels
    for (const auto& s : sim.shovels)
    {
        Vector2 sp = worldToScreen(s.GetPosition());
        DrawCircleV(sp, 12, YELLOW);
        DrawTextEx(font, ("S" + std::to_string(s.GetId())).c_str(), {sp.x + 16, sp.y - 8}, 18, 1, WHITE);

        int truckCount = 0;
        for (const TruckId t : s.GetQueue())
        {
            if (sim.trucks[t.value].GetState() == TruckState::Travelling) continue;
            Vector2 tp = worldToScreen(sim.trucks[t.value].GetPosition());
            RenderTruck(sim.trucks[t.value], tp, tp.x + (22 * truckCount), tp.y - 30, font);
            truckCount++;
        }
    }

    // Dumps
    for (const auto& d : sim.dumps)
    {
        Vector2 dp = worldToScreen(d.GetPosition());
        DrawRectangle((int)dp.x - 12, (int)dp.y - 12, 24, 24, RED);
        DrawTextEx(font, ("D" + std::to_string(d.GetId())).c_str(), {dp.x + 14, dp.y - 8}, 18, 1, WHITE);

        int truckCount = 0;
        for (const TruckId t : d.GetQueue())
        {
            if (sim.trucks[t.value].GetState() == TruckState::Travelling) continue;
            Vector2 tp = worldToScreen(sim.trucks[t.value].GetPosition());
            RenderTruck(sim.trucks[t.value], tp, tp.x + (22 * truckCount), tp.y - 30, font);
            truckCount++;
        }
    }

    // Trucks - mainly rendered by sites, but this is for the ones that are between sites
    
    for (const auto& t : sim.trucks)
    {
        if (t.GetState() != TruckState::Travelling && t.GetState() != TruckState::Broken) continue;
        
        Vector2 start = worldToScreen(t.GetPosition());
        Vector2 end = worldToScreen(t.targetPosition);

        float perc = t.EstTaskCompletionPercentage(sim.currentTime);
        Position truckPos = t.GetPosition();
        Position mid = truckPos + (t.targetPosition - truckPos) * perc;
        Vector2 tp = worldToScreen(mid);
        
        RenderTruck(t, tp, tp.x, tp.y - 30, font);

        // Indicate direction of travel
        DrawLine(start.x, start.y, tp.x, tp.y, RED);
        DrawLine(tp.x, tp.y, end.x, end.y, YELLOW);
    }
    
    
    // Sim time
    DrawText(("t=" + std::to_string(static_cast<int>(sim.currentTime))).c_str(), 10,
10, 20, WHITE);

    std::string evtLabel = evt.truck.value >= 0
          ? "T" + std::to_string(sim.trucks[evt.truck.value].GetId()) + ": " + EventTypeToString(evt.type)
          : "Waiting...";
    DrawText(evtLabel.c_str(), 10, 35, 20, WHITE);

    if (sim.isPaused)
    {
        // Key prompt
        DrawText("Press the space key to resume...", 10, 578, 16, DARKGRAY);
    }
    else
    {
        DrawText("Press the space key to pause...", 10, 578, 16, DARKGRAY);
    }
}



