# Raylib Step-Through Renderer

## Goal
Add a 2D top-down render window using raylib that updates after each sim event and waits for a keypress before continuing — allowing visual inspection of truck/shovel/dump state at each step.

---

## Step 1: Install raylib

Use vcpkg:
```
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install
vcpkg install raylib:x64-windows
```

Or manual: download prebuilt binaries from github.com/raysan5/raylib/releases and configure:
- **Additional Include Directories**: path to raylib `include/`
- **Additional Library Directories**: path to raylib `lib/`
- **Additional Dependencies**: `raylib.lib`, `winmm.lib`, `gdi32.lib`, `opengl32.lib`

---

## Step 2: Create `TruckSim/Core/Renderer.h`

### `worldToScreen`
Convert world coordinates to screen pixels. Flip Y axis (screen Y increases downward).

```cpp
#include "raylib.h"
#include "../Types/Position.h"

inline Vector2 worldToScreen(Position pos, float scale = 20.f, Vector2 offset = {400, 300})
{
    return { offset.x + pos.x * scale, offset.y - pos.y * scale };
}
```

### `Render`
Wraps `BeginDrawing`/`EndDrawing`. Draws:
- **Origin**: crosshair at worldToScreen({0,0})
- **Shovels**: YELLOW circle + "S{id}" label
- **Dumps**: RED rectangle + "D{id}" label
- **Trucks**: small circle, colour by TruckState:
  - Idle / Travelling → WHITE
  - Loading → YELLOW
  - Queueing → ORANGE
  - Dumping → RED
- **Sim time**: top-left corner text ("Time: {currentTime}")

```cpp
void Render(const std::vector<Truck>& trucks,
            const std::vector<Shovel>& shovels,
            const std::vector<Dump>& dumps,
            double currentTime);
```

### `WaitForKeypress`
Polls input until any key is pressed or window is closed. Does not redraw — last frame stays visible.

```cpp
bool WaitForKeypress()
{
    while (!WindowShouldClose())
    {
        PollInputEvents();
        if (GetKeyPressed() != 0) return true;
    }
    return false;  // window was closed
}
```

---

## Step 3: Modify `TruckSim/TruckSim.cpp`

### Add includes
```cpp
#include "raylib.h"
#include "Core/Renderer.h"
```

### Initialise window before sim loop
```cpp
InitWindow(800, 600, "TruckSim");
```

### After switch block, render and wait
```cpp
        } // end switch

        if (!WindowShouldClose())
        {
            Render(trucks, shovels, dumps, currentTime);
            if (!WaitForKeypress()) break;
        }
    } // end while

    CloseWindow();
```

### Remove placeholder echo loop
Delete the `while (state.running)` echo block at the bottom.

---

## Files to create / modify

| File | Action |
|---|---|
| `Core/Renderer.h` | Create — worldToScreen, Render, WaitForKeypress |
| `TruckSim.cpp` | Modify — InitWindow, render/wait calls, CloseWindow, remove echo loop |

---

## Verification
1. Build succeeds with raylib linked
2. Window opens showing shovels and dumps at correct positions relative to origin
3. Console continues to log events
4. Each keypress advances one event, window updates truck positions/states
5. Closing the window exits the sim cleanly
