cmake_minimum_required(VERSION 3.16)
project(ElectricGuitar3D)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLM (for 3D math)
find_package(glm REQUIRED)

# GLEW for OpenGL extensions
find_package(GLEW REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

# Add tinygltf as header-only library
add_subdirectory(third_party/tinygltf)

# Source files
set(SOURCES
    src/main.cpp
    src/Guitar3D.cpp
    src/AudioManager.cpp
    src/GLBLoader.cpp
    src/Renderer3D.cpp
    src/Camera.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES} 
    ${SDL2_MIXER_LIBRARIES}
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    tinygltf
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_MIXER_CFLAGS_OTHER}) 