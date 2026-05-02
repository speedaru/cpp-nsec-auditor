cmake_minimum_required(VERSION 3.20)
project(nsec_core LANGUAGES CXX)

# statically link libraries, because on windows it uses git bash to execute
# git hooks and it can't find certain dlls, so we need to statically link them
if(MSVC)
    # for MSVC (Visual Studio / cl.exe)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    message(STATUS "Configuring MSVC for Static Runtime linking (/MT)")
else()
    # for GCC (mingw) or clang
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    message(STATUS "Configuring GCC/Clang for Static linking")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "src/*.hpp"
    "vendor/*.cpp" 
    "vendor/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

# precompiled headers
target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE 
    include 
    src
    vendor
)
