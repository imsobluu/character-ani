# CMakeLists.txt for gen3dart standalone OpenGL project
cmake_minimum_required(VERSION 3.10)
project(gen3dart)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)

# Generate root_directory.h (used by learnopengl/filesystem.h)
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" LOGL_ROOT_PATH)
file(WRITE ${CMAKE_SOURCE_DIR}/includes/learnopengl/root_directory.h
"const char* logl_root = \"${LOGL_ROOT_PATH}\";")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/includes)

# Find OpenGL
find_package(OpenGL REQUIRED)

# GLFW library
set(GLFW_LIB ${CMAKE_SOURCE_DIR}/lib/glfw3.lib)

# Source files
set(GLAD_SRC ${CMAKE_SOURCE_DIR}/src/glad.c)

# skeletal_animation executable
add_executable(skeletal_animation
    src/8.guest/2020/skeletal_animation/skeletal_animation.cpp
    src/stb_image.cpp
    ${GLAD_SRC}
)

target_link_libraries(skeletal_animation
    ${OPENGL_LIBRARIES}
    ${GLFW_LIB}
    ${CMAKE_SOURCE_DIR}/lib/assimp.lib
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(skeletal_animation
        user32
        gdi32
        shell32
    )
    if(MSVC)
        # Suppress LNK4099 warnings about missing PDB files for pre-compiled libraries
        target_link_options(skeletal_animation PRIVATE "/ignore:4099")
    endif()
endif()
