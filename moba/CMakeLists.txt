find_package(TinyXML REQUIRED)

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
include_directories(${TINYXML_INCLUDE_DIR})
link_directories(${TINYXML_LIBRARY_DIR})
include_directories(${TINY_SOURCE_DIR})
include_directories(${TINY_BINARY_DIR})

set(MOBA_LIBS 
              tinygame
              ${ASSIMP_LIBRARIES}
              ${OPENGL_LIBRARY}
              ${OPENAL_LIBRARY}
              ${OGGVORBIS_LIBRARIES}
              ${GLEW_LIBRARY}
              ${SDL_LIBRARY}
              ${SDLIMAGE_LIBRARY}
              ${SDLTTF_LIBRARY}
              ${SDLNET_LIBRARY}
              ${TINYXML_LIBRARIES})

add_executable(moba src/moba.cpp
                    src/terrain.cpp
                    src/forest.cpp
                    src/minion_type.cpp
                    src/game.cpp)

target_link_libraries(moba ${MOBA_LIBS})

