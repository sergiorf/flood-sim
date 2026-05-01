cmake_minimum_required(VERSION 3.20)

project(FloodSim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FLOODSIM_BUILD_TESTS "Build FloodSim tests" ON)

if(FLOODSIM_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(core)

add_executable(floodsim_simple_grid
    examples/simple_grid/main.cpp
)

target_link_libraries(floodsim_simple_grid
    PRIVATE
        floodsim_core
)

target_compile_features(floodsim_simple_grid PRIVATE cxx_std_20)
