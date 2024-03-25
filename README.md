# 3VG3 Physics Engine

A physics engine based on my coursework in DATASCI 3VG3 at McMaster University. 
The course goes over 2D dynamics, collisions, friction, constraints, and more all in Python,
I translated everything over to C++ (using Raylib for rendering) with my own engine additions 
beyond the scope of the course.

### TODO
- Friction
- Sequential impulse solver
- Constraints

Things beyond the scope of the course that will probably take me much more time:
- 3D (oh boy)
- Different collider types (OBB, sphere, convex hull, etc.)


## Building

This project uses CMake, my dastardly arch-nemesis. It's been a tough process learning how
to build things with CMake, but in the end it's really useful because now I don't need to
include a lot of dependencies in the source (except imgui).

First, clone the repository:
```
git clone https://github.com/plagakit/3VG3.git
cd 3VG3
```

To build for desktop, run:
```
mkdir bin
cd bin
cmake ..
```
And then build the project with your desktop's build system.

To build for web using emscripten, run: (make sure CMake version is 3.13 or greater)
```
mkdir bin
cd bin
cmake -S .. -B build -G Ninja -DPLATFORM=Web "-DCMAKE_TOOLCHAIN_FILE=<YOUR EMSCRIPTEN PATH>/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
cmake --build build
```