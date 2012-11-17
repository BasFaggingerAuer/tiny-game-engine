include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${SDL_IMAGE_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_library(tinygame
            draw/buffer.cpp
            draw/computetexture.cpp
            draw/renderable.cpp
            draw/renderer.cpp
            draw/shader.cpp
            draw/shaderprogram.cpp
            draw/texture.cpp
            draw/vertexbufferinterpreter.cpp)

