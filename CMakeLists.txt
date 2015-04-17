#Copyright 2012, Bas Fagginger Auer.
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 2.6)
project(TINY)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TINY_SOURCE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(SDL_net REQUIRED)
find_package(Assimp REQUIRED)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -ansi -pedantic")
#set(CMAKE_CXX_FLAGS "-O3 -Wall -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-lrt")
#set(CMAKE_VERBOSE_MAKEFILE true)

add_subdirectory(${TINY_SOURCE_DIR}/tiny/)

include_directories(${OPENGL_INCLUDE_DIR})
link_directories(${OPENGL_LIBRARY_DIR})
include_directories(${GLEW_INCLUDE_DIR})
link_directories(${GLEW_LIBRARY_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${OGGVORBIS_INCLUDE_DIR})
include_directories(${ASSIMP_INCLUDE_DIR})
link_directories(${ASSIMP_LIBRARY_DIR})
include_directories(${SDL_INCLUDE_DIR})
link_directories(${SDL_LIBRARY_DIR})
include_directories(${SDL_IMAGE_INCLUDE_DIR})
link_directories(${SDL_IMAGE_LIBRARY_DIR})
include_directories(${SDL_TTF_INCLUDE_DIR})
link_directories(${SDL_TTF_LIBRARY_DIR})
include_directories(${SDL_NET_INCLUDE_DIR})
link_directories(${SDL_NET_LIBRARY_DIR})
include_directories(${TINY_SOURCE_DIR})
include_directories(${TINY_BINARY_DIR})

set(USED_LIBS 
              tinygame
              ${ASSIMP_LIBRARIES}
              ${OPENGL_LIBRARY}
              ${GLEW_LIBRARY}
              ${OPENAL_LIBRARY}
              ${OGGVORBIS_LIBRARIES}
              ${SDL_LIBRARY}
              ${SDLIMAGE_LIBRARY}
              ${SDLTTF_LIBRARY}
              ${SDLNET_LIBRARY})

add_executable(test_SDLApplication src/test_SDLApplication.cpp)
target_link_libraries(test_SDLApplication ${USED_LIBS})

add_executable(test_ComputeTexture src/test_ComputeTexture.cpp)
target_link_libraries(test_ComputeTexture ${USED_LIBS})

add_executable(test_Font src/test_Font.cpp)
target_link_libraries(test_Font ${USED_LIBS})

add_executable(test_Console src/test_Console.cpp)
target_link_libraries(test_Console ${USED_LIBS})

add_executable(test_StaticMesh src/test_StaticMesh.cpp)
target_link_libraries(test_StaticMesh ${USED_LIBS})

add_executable(test_SoundSource src/test_SoundSource.cpp)
target_link_libraries(test_SoundSource ${USED_LIBS})

add_executable(test_ReadStaticMesh src/test_ReadStaticMesh.cpp)
target_link_libraries(test_ReadStaticMesh ${USED_LIBS})

add_executable(test_ReadSample src/test_ReadSample.cpp)
target_link_libraries(test_ReadSample ${USED_LIBS})

add_executable(test_ReadAnimatedMesh src/test_ReadAnimatedMesh.cpp)
target_link_libraries(test_ReadAnimatedMesh ${USED_LIBS})

add_executable(test_StaticMeshHorde src/test_StaticMeshHorde.cpp)
target_link_libraries(test_StaticMeshHorde ${USED_LIBS})

add_executable(test_AnimatedMeshHorde src/test_AnimatedMeshHorde.cpp)
target_link_libraries(test_AnimatedMeshHorde ${USED_LIBS})

add_executable(test_WorldIconHorde src/test_WorldIconHorde.cpp)
target_link_libraries(test_WorldIconHorde ${USED_LIBS})

add_executable(test_Terrain src/test_Terrain.cpp)
target_link_libraries(test_Terrain ${USED_LIBS})

add_executable(test_TerrainFar src/test_TerrainFar.cpp)
target_link_libraries(test_TerrainFar ${USED_LIBS})

add_executable(test_Quadtree src/test_Quadtree.cpp)
target_link_libraries(test_Quadtree ${USED_LIBS})

add_executable(test_TerrainFancy src/test_TerrainFancy.cpp)
target_link_libraries(test_TerrainFancy ${USED_LIBS})

add_executable(test_Network src/test_Network.cpp)
target_link_libraries(test_Network ${USED_LIBS})


add_subdirectory(${TINY_SOURCE_DIR}/tanks/)

add_subdirectory(${TINY_SOURCE_DIR}/minions/)

