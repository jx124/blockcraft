# Blockcraft
This is my attempt at making a multiplayer Minecraft clone to learn more about computer graphics, game development, and networking.

## Quick Start
On Linux:
- Run `chmod +x ./build.sh` to add execution permissions to the build script.
- Run `./build.sh -r` to build the game in release mode (use `-d` instead for debug mode).
- Run the executable in the directory `./build/release/` (or `./build/debug`).

Manually using CMake:
- Run `cmake -B build/release -DCMAKE_BUILD_TYPE=release` to set the build binary directory.
- Run `cmake --build ./build/release -j $(nproc)` to build the executable. Replace `$(nproc)` with the number of cores desired for the build if needed.
- Run the executable in the directory `./build/release/`.
Replace `release` with `debug` for the debug build.

## Acknowledgements
- The Entity Component System (ECS) code mainly follows this tutorial by Alex Jobe: https://www.alexjobe.net/posts/ecs with some modifications. Namely, I changed the `shared_ptr`s to `unique_ptr`s since the lifetime of the resources are taken care of by the ECS. I also used fold expressions to allow registering multiple Components to one System at a time
