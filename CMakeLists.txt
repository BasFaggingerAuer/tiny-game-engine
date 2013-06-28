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
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -ansi -pedantic")
#set(CMAKE_CXX_FLAGS "-O3 -Wall -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-lrt")

add_subdirectory(${TINY_SOURCE_DIR}/tiny/)

include_directories(${OPENGL_INCLUDE_DIR})
link_directories(${OPENGL_LIBRARY_DIR})
include_directories(${GLEW_INCLUDE_DIR})
link_directories(${GLEW_LIBRARY_DIR})
include_directories(${SDL_INCLUDE_DIR})
link_directories(${SDL_LIBRARY_DIR})
include_directories(${SDL_IMAGE_INCLUDE_DIR})
link_directories(${SDL_IMAGE_LIBRARY_DIR})
include_directories(${SDL_TTF_INCLUDE_DIR})
link_directories(${SDL_TTF_LIBRARY_DIR})
include_directories(${TINY_SOURCE_DIR})
include_directories(${TINY_BINARY_DIR})

set(USED_LIBS tinygame
              ${OPENGL_LIBRARY}
              ${GLEW_LIBRARY}
              ${SDL_LIBRARY}
              ${SDLIMAGE_LIBRARY}
              ${SDLTTF_LIBRARY})

add_executable(test_SDLApplication src/test_SDLApplication.cpp)
target_link_libraries(test_SDLApplication ${USED_LIBS})

add_executable(test_ComputeTexture src/test_ComputeTexture.cpp)
target_link_libraries(test_ComputeTexture ${USED_LIBS})

add_executable(test_Font src/test_Font.cpp)
target_link_libraries(test_Font ${USED_LIBS})

add_executable(test_StaticMesh src/test_StaticMesh.cpp)
target_link_libraries(test_StaticMesh ${USED_LIBS})

add_executable(test_StaticMeshHorde src/test_StaticMeshHorde.cpp)
target_link_libraries(test_StaticMeshHorde ${USED_LIBS})

add_executable(test_Terrain src/test_Terrain.cpp)
target_link_libraries(test_Terrain ${USED_LIBS})

add_executable(test_TerrainFar src/test_TerrainFar.cpp)
target_link_libraries(test_TerrainFar ${USED_LIBS})

add_executable(test_Quadtree src/test_Quadtree.cpp)
target_link_libraries(test_Quadtree ${USED_LIBS})

add_executable(test_world src/test_world.cpp)
target_link_libraries(test_world ${USED_LIBS})

