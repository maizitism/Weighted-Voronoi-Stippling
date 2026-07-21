# Weighted Voronoi Stippling

A C++ implementation of weighted Voronoi stippling. See [notes.md](notes.md) for
background on the technique.

## Prerequisites (per machine)

- **Visual Studio** with the *Desktop development with C++* workload (provides
  the MSVC compiler).
- **CMake 3.21+** (bundled with Visual Studio, or `winget install Kitware.CMake`).
- **CLion** (recommended IDE), or any CMake-aware editor. A terminal + CMake
  alone also works.

That's it. The project pins no generator and uses no absolute paths, so CMake
picks each machine's newest Visual Studio automatically and builds unchanged.

## Building

### From CLion (recommended)

CLion reads `CMakePresets.json` natively, so no extra setup is needed:

1. **File -> Open** the project folder. When prompted, let CLion enable the
   **CMake presets** it finds (the `default` / MSVC preset).
2. Make sure the CMake toolchain is **Visual Studio** (*Settings -> Build,
   Execution, Deployment -> Toolchains*). This project uses MSVC-only flags
   (`/W4 /EHsc`), so the bundled MinGW toolchain will not compile it.
3. Switch between **Debug** and **Release** with the build-type selector next to
   the run/build buttons.
4. A shared **stippling** run configuration is committed in `.run/`. It passes
   the required `--i` / `--o` arguments (`--i src/image.jpg --o stipples.bmp`)
   and runs from the project root, so **Run**/**Debug** works out of the box.
   Edit its *Program arguments* to point at a different image.

> The program requires `--i <input>` and `--o <output>`; running with no
> arguments exits immediately. That is why the run configuration above exists.

### From a terminal

```
cmake --preset default        # configure (generates build/ once)
cmake --build --preset debug  # compile -> build/Debug/stippling.exe
cmake --build --preset release
```

Run it:

```
build\Debug\stippling.exe
```

## Layout

```
src/               C++ sources (add your .cpp / .h files here)
build/             Generated build tree (git-ignored)
CMakeLists.txt     Project definition
CMakePresets.json  Configure/build presets (Debug, Release)
.run/              Shared CLion run configuration (stippling, with args)
documents/         Reference papers
```

The output binary is `build/Debug/stippling.exe` (or `build/Release/...`).
