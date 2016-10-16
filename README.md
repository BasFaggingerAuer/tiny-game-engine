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
3.   OpenGL libraries.
4.   GLEW OpenGL Extension Wrangler Library (libglew-dev).
5.   OpenAL library (libopenal-dev).
6.   OGG Vorbis library (libogg-dev, libvorbis-dev).
7.   SDL 2 Simple DirectMedia Layer (libsdl2-dev).
8.   SDL 2 image library (libsdl2-image-dev).
9.   SDL 2 font library (libsdl2-ttf-dev).
10.  SDL 2 networking library (libsdl2-net-dev).
11.  Asset importer library (libassimp-dev).
12.  Tiny XML library (libtinyxml-dev).

Installation
------------

Compiling the tiny game engine is done using CMake.
Please open a terminal and go to the directory where the package was extracted (containing this README).
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

*   [test_SDLApplication](/src/test_SDLApplication.cpp): Create an empty SDL window.
*   [test_ComputeTexture](/src/test_ComputeTexture.cpp): Example in which a test image is passed through a simple GLSL fragment shader to the screen.
*   [test_Font](/src/test_Font.cpp): Read a TrueType font and display text on the screen.
*   [test_Console](/src/test_Console.cpp): Provide a console to provide direct commands to the engine.
*   [test_StaticMesh](/src/test_StaticMesh.cpp): Render a cube on the screen, viewed through a controllable camera.
*   [test_SoundSource](/src/test_SoundSource.cpp): Play a 440Hz test tone.
*   [test_ReadStaticMesh](/src/test_ReadStaticMesh.cpp): Render a mesh read from disk on the screen, viewed through a controllable camera.
*   [test_ShaderHashing](/src/test_ShaderHashing.cpp): Test hashing functionality of shader programs for multiple different meshes rendered with the same shader program.
*   [test_ReadAnimatedMesh](/src/test_AnimatedMesh.cpp): Read and render an animated mesh using skeletal animation.
*   [test_ReadSample](/src/test_ReadSample.cpp): Read and play an OGG audio sample.
*   [test_StaticMeshHorde](/src/test_StaticMeshHorde.cpp): Render a large number of cube instances on the screen, viewed through a controllable camera.
*   [test_AnimatedMeshHorde](/src/test_AnimatedMeshHorde.cpp): Same as above, but with skeletal animation.
*   [test_Terrain](/src/test_Terrain.cpp): Fly over a simple terrain.
*   [test_TerrainFar](/src/test_TerrainFar.cpp): Fly over a very large terrain.
*   [test_Quadtree](/src/test_Quadtree.cpp): Example of using a quadtree for level of detail management.
*   [test_TerrainFancy](/src/test_TerrainFancy.cpp): Fly over a very large terrain with advanced texturing, atmospherics, and a forest.
*   [test_Network](/src/test_Network.cpp): Basic networking functionality.
*   [test_WorldIconHorde](/src/test_WorldIconHorde.cpp): Draw a large number of player-facing sprites.

