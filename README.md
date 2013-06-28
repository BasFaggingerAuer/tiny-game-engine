tiny-game-engine
================

A small OpenGL game engine, created by Bas Fagginger Auer.

Copyright
---------

The tiny game engine is distributed under the GNU General Public License version 3, please see COPYING.

Requirements
------------

Before compiling this software, please ensure that the following tools are available on your system.
Debian/Ubuntu package names are supplied between parenthesis for convenience.

1.   A sane build environment (build-essential, g++).
2.   CMake (cmake).
3.   OpenGL development libraries.
4.   GLEW OpenGL Extension Wrangler Library (libglew-dev).
5.   SDL Simple DirectMedia Layer (libsdl1.2-dev).
6.   SDL image library (libsdl-image1.2-dev).
7.   SDL font library (libsdl-ttf2.0-dev).

Installation
------------

Compiling the tiny game engine is done using CMake.
Please open a terminal and go to the directory where the source code was extracted (containing this README).
Then, issue the following commands:

1.   mkdir build
2.   cd build
3.   cmake ..
4.   make

This will automatically build the tiny game engine library (libtinygame), as well as tests for this library's functionality.

Using libtinygame
-----------------

To familiarise yourself with the tiny game engine, I would recommend looking at the source code of the provided examples in the [src](/src/) directory.
Below is a list of examples, ordered by complexity, together with the particular elements that they are designed to test.

1.   [test_SDLApplication](/src/test_SDLApplication.cpp): Create an empty SDL window.
2.   [test_ComputeTexture](/src/test_ComputeTexture.cpp): Example in which a test image is passed through a simple GLSL fragment shader to the screen.
3.   [test_Font](/src/test_Font.cpp): Read a TrueType font and display text on the screen.
4.   [test_StaticMesh](/src/test_StaticMesh.cpp): Render a cube on the screen, viewed through a controllable camera.
5.   [test_StaticMeshHorde](/src/test_StaticMeshHorde.cpp): Render a large number of cube instances on the screen, viewed through a controllable camera.
6.   [test_Terrain](/src/test_Terrain.cpp): Fly over a simple terrain.
7.   [test_TerrainFar](/src/test_TerrainFar.cpp): Fly over a very large terrain.
8.   [test_TerrainFancy](/src/test_TerrainFancy.cpp): Fly over a very large terrain with advanced texturing and atmospherics.
9.   [test_Quadtree](/src/test_Quadtree.cpp): Example of using a quadtree for level of detail management.

