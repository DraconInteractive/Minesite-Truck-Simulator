# TruckSim

A discrete-event simulation of a mining haul truck fleet. Trucks cycle between shovel loading sites and dump sites, queuing when sites are busy. The simulation is driven entirely by an event queue — no fixed time steps.

## Features

- Discrete-event simulation engine with a priority queue scheduler
- Trucks route to the best available shovel or dump based on distance and queue length
- Step-through rendering via raylib — press any key to advance to the next event
- JSON config file for defining sites, trucks, seed events, routing weights, and debug options
- Per-truck task tracking with estimated completion percentage

## Architecture

```
TruckSim/
├── Core/
│   ├── Config.h/.cpp       — JSON config loading, SimState construction
│   ├── Event.h             — Event struct and EventType enum
│   ├── Simulation.h/.cpp   — Event handlers and dispatch helpers
│   └── Utilities.h         — Travel time calculation
├── Entities/
│   ├── Mobile/
│   │   └── Truck.h         — Truck state, load tracking, task progress
│   └── Stationary/
│       ├── Shovel.h/.cpp   — Shovel site, loading queue
│       └── Dump.h/.cpp     — Dump site, dumping queue
├── Rendering/
│   └── Renderer.h          — raylib rendering (sites, trucks, travel lines)
├── Types/
│   ├── Position.h          — float x/y with + and * operators
│   ├── RoutingConfig.h     — distance/queue priority weights
│   ├── DebugConfig.h       — logEvents, timeCap
│   └── SimState.h          — central simulation state struct
└── sim.json                — simulation configuration
```

## Simulation Loop

Each iteration pops the earliest event, advances `currentTime` to that event's time, and dispatches to the relevant handler:

| Event | Handler |
|---|---|
| `TruckArriveShovel` | Enqueue truck, start loading or set Queueing |
| `TruckFinishLoading` | Fill truck, dispatch to best dump, start next in queue |
| `TruckArriveDump` | Enqueue truck, start dumping or set Queueing |
| `TruckFinishDumping` | Empty truck, dispatch to best shovel, start next in queue |

## Configuration (`sim.json`)

```json
{
  "trucks":     [{ "id": 0, "speed": 30, "capacity": 400 }],
  "shovels":    [{ "id": 0, "x": 5, "y": 10, "loadSpeed": 120 }],
  "dumps":      [{ "id": 0, "x": -3, "y": 6, "dumpSpeed": 160 }],
  "seedEvents": [{ "truckId": 0, "shovelId": 0, "arrivalTime": 2 }],
  "routing":    { "distancePriority": 1.0, "queuePriority": 1.0 },
  "debug":      { "logEvents": true, "timeCap": 1000 }
}
```

- **speed**: km/h
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

Travelling trucks are rendered at their midpoint with a red line (from) and yellow line (to). Shovels are yellow circles, dumps are red squares.

## Dependencies

- [raylib](https://www.raylib.com/) — rendering
- [nlohmann/json](https://github.com/nlohmann/json) — config parsing

Both managed via vcpkg.
