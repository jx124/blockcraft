# This file was adapted from the GLFW-CMake-starter CMakeLists.txt file 
# from https://github.com/enkisoftware/GLFW-CMake-starter. The original 
# file was licensed under the zlib license:
#
# Copyright (c) 2021 Juliette Foucaut and Doug Binks
# 
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

cmake_minimum_required(VERSION 3.22)
project(blockcraft)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Weffc++)

# TODO: set compile flags based on compiler
if(CMAKE_BUILD_TYPE STREQUAL "debug")
    add_compile_definitions(DEBUG=1)
    add_compile_options(-Og -march=native)
else(CMAKE_BUILD_TYPE STREQUAL "release")
    add_compile_options(-O2 -march=native)
endif()

find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")
set(GLAD_GL "")

add_subdirectory(external/glfw)

include_directories("${GLFW_SOURCE_DIR}/deps")
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h")

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

add_subdirectory(external/glm)

set(GLM_BUILD_TESTS OFF CACHE BOOL "GLM lib only")

add_executable(blockcraft WIN32 main.cpp ${GLAD_GL})
target_link_libraries(blockcraft ${OPENGL_LIBRARIES} glfw glm::glm)

add_subdirectory(src)
add_subdirectory(include)

if(MSVC)
    if(${CMAKE_VERSION} VERSION_LESS "3.22.0")
        message("\n\t[ WARNING ]\n\n\tCMake version lower than 3.22.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'GLFW-CMake-starter' as StartUp Project in Visual Studio.\n")
    else()
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT blockcraft)
    endif()
endif()
