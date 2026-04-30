cmake_minimum_required(VERSION 3.20)
project(nsec-auditor LANGUAGES CXX)

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
