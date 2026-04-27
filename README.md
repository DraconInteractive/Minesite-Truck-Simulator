# TruckSim

<video src="docs/videos/TruckSim_Recording_270426_01.mp4" controls width="800"></video>

A discrete-event simulation of a mining haul truck fleet. Trucks cycle between shovel loading sites and dump sites, queuing when sites are busy. The simulation is driven entirely by an event queue — no fixed time steps.

## Features

- Discrete-event simulation engine with a priority queue scheduler
- Trucks route to the best available shovel or dump based on distance and estimated wait time
- Variable truck speed — interpolated between unladen and laden speed based on current fill level
- Parts-based truck loadout system — trucks carry named components (axle, tires, hydraulics) with health, wear, and repair times
- Real-time rendering via raylib — pause/resume with spacebar; travelling trucks interpolated along road paths
- Node-graph road network — sites connect via weighted edges; Dijkstra pathfinding drives all dispatch routing
- Simulation statistics tracked live: units loaded, units dumped, total time spent travelling
- JSON config file for defining sites, trucks, nodes, edges, seed events, routing weights, and debug options
- Loadout files — separate JSON files define part configurations, referenced by trucks in `sim.json`
- Per-truck task tracking with estimated completion percentage

## Architecture

```
TruckSim/
├── Core/
│   ├── Config.h/.cpp       — JSON config loading, SimState construction, loadout file resolution
│   ├── Event.h             — Event struct and EventType enum
│   ├── Simulation.h/.cpp   — Event handlers and dispatch helpers
│   └── Utilities.h         — Lerp
├── Entities/
│   ├── Mobile/
│   │   ├── MobileEntity.h  — Base class with virtual GetSpeed()
│   │   └── Truck.h/.cpp    — Truck state, load tracking, variable speed, parts, RollForFailure, path storage
│   └── Stationary/
│       ├── StationaryEntity.h/.cpp — Base class: queue, TimeToProcess (virtual), GetBestSite<T>, closestNodeId
│       ├── Shovel.h/.cpp   — Shovel site, GetBestShovel wrapper
│       └── Dump.h/.cpp     — Dump site, GetBestDump wrapper
├── Navigation/
│   ├── Node.h              — Node struct (id, x, y, connectedEdges)
│   ├── Edge.h              — Edge struct (id, fromNode, toNode, speedMultiplier)
│   └── Pathfinding.h       — Dijkstra (GetPathById, GetPathByPosition), GetPositionAlongPath
├── Rendering/
│   └── Renderer.h          — raylib rendering: nodes, edges, sites, trucks with path interpolation, HUD stats
├── Types/
│   ├── EntityPart.h        — EntityPart struct (name, health, repairTime, baseFailRate, wearPerCycle)
│   ├── Position.h          — float x/y with +, * operators
│   ├── RoutingConfig.h     — distance/queue priority weights
│   ├── DebugConfig.h       — logEvents, timeCap, pauseOnFail
│   └── SimState.h          — central state: trucks, shovels, dumps, nodes, edges, stats, event queue
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
  "nodes":      [{ "id": 0, "x": 0, "y": 0 }],
  "edges":      [{ "id": 0, "fromId": 0, "toId": 1, "speedMult": 1.0 }],
  "routing":    { "distancePriority": 1.0, "queuePriority": 1.0 },
  "debug":      { "logEvents": true, "timeCap": 1000, "pauseOnFail": false }
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

## Navigation

All routing uses a node-graph defined in `sim.json`. Each node has an `(x, y)` position; edges connect pairs of nodes with an optional `speedMultiplier`. Dispatch calls `GetPathByPosition`, which snaps positions to the nearest node and runs Dijkstra to find the lowest-time path.

Shovels and dumps cache their closest node at startup — used as path endpoints when dispatching trucks. Travelling trucks store the full node sequence and interpolate their rendered position along it using time-weighted segment progress.

## Statistics

The following counters are tracked in `SimState` and displayed on the HUD:

| Field | Description |
|---|---|
| `totalUnitsLoaded` | Cumulative units loaded across all shovel visits |
| `totalUnitsDumped` | Cumulative units dumped across all dump visits |
| `totalTimeSpentTravelling` | Sum of all travel legs (minutes), including legs cut short by breakdowns |

## Rendering

The renderer runs in real-time at 60 FPS. Press **spacebar** to pause or resume. All rendering happens between `BeginDrawing` / `EndDrawing` in the main loop.

Trucks are colour-coded by state:

| Colour | State |
|---|---|
| Gray | Idle |
| Sky Blue | Travelling |
| Green | Loading |
| Orange | Queueing |
| Purple | Dumping |
| Red | Broken |

Road nodes are drawn as small light-grey circles; edges as light-grey lines. Shovels are yellow circles labelled `S0`, `S1`, …; dumps are red squares labelled `D0`, `D1`, …. Travelling trucks are rendered at their interpolated position along their path. Broken trucks are rendered at their last known position. The HUD (top-left) shows current sim time, last event, and live stat counters.

## Dependencies

- [raylib](https://www.raylib.com/) — rendering
- [nlohmann/json](https://github.com/nlohmann/json) — config parsing

Both managed via vcpkg.
