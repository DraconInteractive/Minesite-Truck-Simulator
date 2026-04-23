# TruckSim

A discrete-event simulation of a mining haul truck fleet. Trucks cycle between shovel loading sites and dump sites, queuing when sites are busy. The simulation is driven entirely by an event queue — no fixed time steps.

## Features

- Discrete-event simulation engine with a priority queue scheduler
- Trucks route to the best available shovel or dump based on distance and estimated wait time
- Variable truck speed — interpolated between unladen and laden speed based on current fill level
- Parts-based truck loadout system — trucks carry named components (axle, tires, hydraulics) with health, wear, and repair times
- Step-through rendering via raylib — press any key to advance to the next event
- JSON config file for defining sites, trucks, seed events, routing weights, and debug options
- Loadout files — separate JSON files define part configurations, referenced by trucks in `sim.json`
- Per-truck task tracking with estimated completion percentage

## Architecture

```
TruckSim/
├── Core/
│   ├── Config.h/.cpp       — JSON config loading, SimState construction, loadout file resolution
│   ├── Event.h             — Event struct and EventType enum
│   ├── Simulation.h/.cpp   — Event handlers and dispatch helpers
│   └── Utilities.h         — Travel time calculation, Lerp
├── Entities/
│   ├── Mobile/
│   │   ├── MobileEntity.h  — Base class with virtual GetSpeed()
│   │   └── Truck.h/.cpp    — Truck state, load tracking, variable speed, parts, RollForFailure
│   └── Stationary/
│       ├── StationaryEntity.h/.cpp — Base class: queue, TimeToProcess (virtual), GetBestSite<T>
│       ├── Shovel.h/.cpp   — Shovel site, GetBestShovel wrapper
│       └── Dump.h/.cpp     — Dump site, GetBestDump wrapper
├── Rendering/
│   └── Renderer.h          — raylib rendering (sites, trucks, travel lines)
├── Types/
│   ├── EntityPart.h        — EntityPart struct (name, health, repairTime, baseFailRate, wearPerCycle)
│   ├── Position.h          — float x/y with +, * operators
│   ├── RoutingConfig.h     — distance/queue priority weights
│   ├── DebugConfig.h       — logEvents, timeCap
│   └── SimState.h          — central simulation state struct
├── sim.json                — simulation configuration
└── loadout_dumptruck_01.json — part loadout for standard dump trucks
```

## Simulation Loop

Each iteration pops the earliest event, advances `currentTime` to that event's time, and dispatches to the relevant handler:

| Event | Handler |
|---|---|
| `TruckEnterSimulation` | Dispatch truck to best shovel |
| `TruckArriveShovel` | Enqueue truck, start loading or set Queueing |
| `TruckFinishLoading` | Fill truck, check part failure, dispatch to best dump, start next in queue |
| `TruckArriveDump` | Enqueue truck, start dumping or set Queueing |
| `TruckFinishDumping` | Empty truck, check part failure, dispatch to best shovel, start next in queue |
| `TruckPartFail` | Set truck Broken, schedule repair completion |
| `TruckPartFixed` | Restore part health, resume dispatch |

## Routing

`GetBestSite<T>` (on `StationaryEntity`) scores each candidate site:

```
score = travelTime * distancePriority + queueTime * queuePriority
```

`queueTime` accounts for:
- Truck currently being served: estimated time remaining (via `EstTaskTimeRemaining`)
- Other queued trucks: full `TimeToProcess` per truck
- En-route trucks arriving before this truck: their remaining travel time + `TimeToProcess`

`GetBestShovel` and `GetBestDump` are thin wrappers that call `GetBestSite` with the typed vector and return a typed ID.

## Parts System

Each truck carries a list of `EntityPart` objects loaded from a loadout file. On every successful dispatch (after `FinishLoading` / `FinishDumping`), `RollForFailure` is called:

- Each part rolls against `baseFailRate / health` — lower health increases failure probability
- If any parts fail, the most damaged one breaks; a `TruckPartFail` event is scheduled
- If no failure, all parts lose `wearPerCycle` health
- On repair, the broken part is restored to full health and the truck resumes

`EntityPart` is defined in `Types/EntityPart.h` and is not truck-specific — shovels, dumps, or other entities can carry parts using the same struct.

## Configuration

### `sim.json`
```json
{
  "trucks":     [{ "id": 0, "ladenSpeed": 17, "unladenSpeed": 35, "capacity": 400, "loadoutFile": "loadout_dumptruck_01.json" }],
  "shovels":    [{ "id": 0, "x": 5, "y": 10, "loadSpeed": 120 }],
  "dumps":      [{ "id": 0, "x": -3, "y": 6, "dumpSpeed": 160 }],
  "seedEvents": [{ "truckId": 0, "arrivalTime": 2 }],
  "routing":    { "distancePriority": 1.0, "queuePriority": 1.0 },
  "debug":      { "logEvents": true, "timeCap": 1000 }
}
```

### Loadout file (`loadout_dumptruck_01.json`)
```json
{
  "name": "Standard Dump Truck",
  "parts": [
    { "name": "Axle",            "repairTime": 180, "baseFailRate": 0.005, "wearPerCycle": 0.006 },
    { "name": "Tires",           "repairTime": 30,  "baseFailRate": 0.010, "wearPerCycle": 0.015 },
    { "name": "Tray Hydraulics", "repairTime": 180, "baseFailRate": 0.002, "wearPerCycle": 0.003 }
  ]
}
```

Multiple loadout files can be defined and referenced by different trucks. Trucks sharing a loadout filename are loaded from the same cached config.

- **ladenSpeed / unladenSpeed**: km/h — actual speed interpolated by fill level
- **loadSpeed / dumpSpeed**: tonnes/minute
- **capacity**: tonnes
- **positions**: km (origin is centre of screen)
- **arrivalTime**: simulated minutes from t=0
- **timeCap**: sim stops after this many minutes

## Rendering

Trucks are colour-coded by state:

| Colour | State |
|---|---|
| Gray | Idle |
| Sky Blue | Travelling |
| Green | Loading |
| Orange | Queueing |
| Purple | Dumping |
| Red | Broken |

Travelling trucks are rendered at their midpoint with a red line (from) and yellow line (to). Shovels are yellow circles, dumps are red squares.

## Dependencies

- [raylib](https://www.raylib.com/) — rendering
- [nlohmann/json](https://github.com/nlohmann/json) — config parsing

Both managed via vcpkg.
