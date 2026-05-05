cmake_minimum_required(VERSION 3.20)

project(FloodSim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FLOODSIM_BUILD_TESTS "Build FloodSim tests" ON)
option(FLOODSIM_ENABLE_GDAL "Enable GDAL-backed terrain ingestion support" ON)

if(FLOODSIM_BUILD_TESTS)
    enable_testing()
    find_package(doctest CONFIG REQUIRED)
    find_package(Python3 COMPONENTS Interpreter)
endif()

if(FLOODSIM_ENABLE_GDAL)
    find_package(GDAL REQUIRED)
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

add_executable(floodsim_real_terrain_example
    examples/real_terrain/main.cpp
)

target_link_libraries(floodsim_real_terrain_example
    PRIVATE
        floodsim_core
)

target_compile_features(floodsim_real_terrain_example PRIVATE cxx_std_20)

if(FLOODSIM_BUILD_TESTS AND Python3_Interpreter_FOUND)
    add_test(
        NAME floodsim_export_consumer_tests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/examples/simple_grid/test_inspect_export.py
    )

    add_test(
        NAME floodsim_real_terrain_example_tests
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/real_terrain/test_real_terrain_example.py
            $<TARGET_FILE:floodsim_real_terrain_example>
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/real_terrain/data/sample_dem.tif
            ${CMAKE_CURRENT_BINARY_DIR}/real_terrain_example/output.csv
    )
endif()
