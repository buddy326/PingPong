# PingPong

A classic Pong game written in C++ with [raylib](https://www.raylib.com/). You play against a CPU-controlled paddle.

**[▶ Play in your browser](https://buddy326.github.io/PingPong/)**

## Controls

| Key | Action |
| --- | --- |
| Click / Enter / Space | Start the game |
| ↑ | Move paddle up |
| ↓ | Move paddle down |
| Esc | Quit |

You control the paddle on the **right**, and the CPU controls the one on the left. Get the ball past the CPU's paddle to score. Each player's score is shown at the top of their half of the screen.

## Building

The project uses [premake5](https://premake.github.io/) to generate build files. The premake binaries and raylib sources live in the `build/` folder. You need a C++ compiler installed (Clang/Xcode Command Line Tools on macOS, GCC on Linux, MinGW-W64 or Visual Studio on Windows).

### macOS

```sh
cd build
./premake5.osx gmake
cd ..
make
```

### Linux

```sh
cd build
./premake5 gmake
cd ..
make
```

### Windows

- **MinGW-W64:** run `build-MinGW-W64.bat`, then run `make` from the project root.
- **Visual Studio 2022:** run `build-VisualStudio2022.bat`, then open the generated `.sln` file.

### VS Code

Open the folder in VS Code and press **Ctrl+Shift+B** to build or **F5** to build and run.

### Web (WebAssembly)

Requires [Emscripten](https://emscripten.org/) (`brew install emscripten` on macOS).

```sh
./build-web.sh
```

This builds raylib for the web the first time, then writes `index.html`, `index.js` and `index.wasm` to `docs/`. GitHub Pages serves that folder as the live demo. To test locally, run `python3 -m http.server -d docs` and open http://localhost:8000.

## Running

The compiled game is written to `bin/Debug/`. On macOS or Linux:

```sh
./bin/Debug/PingPong
```

## Project structure

```
src/main.cpp     Game code: Ball, Paddle, CpuPaddle and the main loop
include/         Helper headers from the template
resources/       Game assets
web/shell.html   HTML page the web build is embedded in (full-window canvas)
docs/            Web build output, served by GitHub Pages
build/           premake5 binaries and build configuration
```

## Credits

Built from [raylib-quickstart](https://github.com/raylib-extras/raylib-quickstart) by Jeffery Myers (CC0 1.0).
