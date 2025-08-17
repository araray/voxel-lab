# Voxel Lab (SDL2/OpenGL 3.3) — a minimal 3D cube framework

A cross-platform **SDL2 + OpenGL** framework to build 3D “voxel-ish” tools on top of cube primitives:

- Sparse, unbounded **universe** in integer (X,Y,Z) coords.
- Choose **edge size in pixels** for the base cube; camera is positioned to match at default zoom.
- **Orbit/pan/zoom/tilt** camera via mouse & keyboard.
- **Right-click** programmable menus (`resources/menus.txt`).
- **Command prompt** at bottom with history and extensible command registry.
- Toggle **grid** and **wireframe**.
- **Selection**, **drag**, **group move**, simple **rotation**.
- Support for **per-cube materials** (solid / gradient) and a hook to **embed custom 3D content** inside a cube.

> This is a **framework** skeleton, intentionally small, readable, and hackable.

## Build

See `USAGE.md` for detailed, OS-specific instructions. TL;DR:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_DEPS=ON
cmake --build build --config Release
./build/voxel_lab
````

## Extending

* Add your own commands via `CommandRegistry::register_cmd`.
* Extend context menu in `resources/menus.txt` (Label=command).
* Attach custom per-cube content by deriving from `IEmbeddedRenderable` and setting `cube.embedded`.
* Replace materials/shaders in `shaders/`.

## License

All code in this repository is MIT-licensed unless third-party libraries specify otherwise.
