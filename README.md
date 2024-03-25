# 3VG3 Physics Engine

A physics engine based on my coursework in DATASCI 3VG3 at McMaster University. 
The course goes over 2D dynamics, collisions, friction, constraints, and more. The course is
normally all in Python, but I translated all the concepts and code over to C++ (using Raylib
for rendering) with, hopefully soon, my own engine additions beyond the scope of the course.

### TODO
- Friction
- Sequential impulse solver
- Constraints

Things beyond the scope of the course that will probably take me much more time:
- 3D (oh boy)
- Different collider types (OBB, sphere, convex hull, etc.)


## Building

This project uses CMake, my dastardly archnemesis. It's been a tough process learning how
to build things with CMake, but worth it in the end because now I don't need to
include a lot of dependencies in the source (except imgui, womp womp).

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
cmake --build build
```

To build for web using emscripten, run: (make sure CMake version is 3.13 or greater)
```
mkdir bin
cd bin
cmake -S .. -B build -G Ninja -DPLATFORM=Web "-DCMAKE_TOOLCHAIN_FILE=<YOUR EMSCRIPTEN PATH>/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
cmake --build build
```
