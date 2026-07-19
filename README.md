# Weighted Voronoi Stippling

A C++ implementation of weighted Voronoi stippling. See [notes.md](notes.md) for
background on the technique.

## Prerequisites (per machine)

- **Visual Studio** with the *Desktop development with C++* workload (provides
  the MSVC compiler).
- **CMake 3.21+** (bundled with Visual Studio, or `winget install Kitware.CMake`).

That's it. The project pins no generator and uses no absolute paths, so CMake
picks each machine's newest Visual Studio automatically and builds unchanged.

## Building

### From VS Code (recommended)

Install the recommended extensions when prompted (CMake Tools + C/C++). Then:

- **Ctrl+Shift+B** builds (Debug).
- **F5** builds and debugs.
- *Terminal -> Run Task* also offers *CMake: build (Release)*, *Run (Debug)*,
  *CMake: clean*.

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
.vscode/           Shared VS Code build / debug / IntelliSense config
documents/         Reference papers
```

The output binary is `build/Debug/stippling.exe` (or `build/Release/...`).
