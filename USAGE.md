# Usage & Controls

## Run
```
voxel\_lab
```

## Controls
- Orbit/tilt: **RMB drag** (or **Alt+LMB drag**)
- Pan: **MMB drag** (or **Shift+RMB drag**)
- Zoom: **Mouse wheel** or `+` / `-`
- Move camera (FPS-style): `W A S D`, up/down: `E`/`Q`
- Select cube: **LMB click** (Ctrl-click to multi-select)
- Drag selection: **LMB drag** (moves along camera plane)
- Rotate selection: hold **R** + mouse X/Y (or `[` / `]` around Y)
- Toggle grid: **G**
- Toggle wireframe: **F2**
- Help overlay: **F1**
- **Right-click** for programmable context menu
- **Prompt** at bottom: type command, press **Enter** (Up/Down to browse history)

## Commands (built-ins)
- `place x y z [color=#RRGGBBAA] [gradient=#..,#..] [dir=x|y|z]`
- `erase x y z` | `erase selection`
- `select x y z` | `select box x1 y1 z1 x2 y2 z2`
- `move dx dy dz` (integers)
- `rotate [x|y|z] degrees`
- `fill solid #RRGGBBAA`
- `fill gradient c1 c2 [dir=x|y|z]`
- `edgepix N` — set base cube edge size in **pixels** (recomputes camera)
- `grid on|off|toggle`
- `wireframe on|off|toggle`
- `group create NAME` | `group select NAME` | `group move NAME dx dy dz` | `group erase NAME`
- `help`

## Programmable Context Menu
Edit `resources/menus.txt`:
```
Label=command string
```

Example:
```
Place at origin=place 0 0 0 color=#66ffffff
Toggle Grid=grid toggle
```

## Design Notes
- **World units**: each cube has **edge = 1 world unit**. `edgepix` decides default camera distance so a unit edge spans *N pixels* on-screen initially; zoom then scales normally.
- **Universe** is sparse (`unordered_map<IVec3,Cube>`), so it grows indefinitely.
- **Renderer** uses instanced drawing for cubes, based on a single unit-cube mesh.
- **Selection** uses ray–AABB picking on integer coordinates and supports group moves/rotations.
- **Embedded content**: implement `IEmbeddedRenderable::draw(model)` and assign to `Cube::embedded`.

## Portability Tips
- If `BUILD_DEPS=OFF`, install SDKs:
  - **Windows (MSVC)**: vcpkg or system packages for `sdl2`, `glew`, `glm`, `imgui`, `catch2`.
  - **Linux**: `sudo apt install libsdl2-dev libglew-dev` (or your distro equivalent).
  - **macOS**: `brew install sdl2 glew glm`

## References

1. SDL 2.30 Documentation — [https://wiki.libsdl.org/SDL2/FrontPage](https://wiki.libsdl.org/SDL2/FrontPage)
2. OpenGL 3.3 Core Profile Spec (Khronos) — [https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf)
3. GLEW — [http://glew.sourceforge.net/](http://glew.sourceforge.net/)
4. GLM (OpenGL Mathematics) — [https://github.com/g-truc/glm](https://github.com/g-truc/glm)
5. Dear ImGui — [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)
6. Catch2 — [https://github.com/catchorg/Catch2](https://github.com/catchorg/Catch2)
