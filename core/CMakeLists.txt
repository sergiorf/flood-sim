add_library(floodsim_core
    src/export.cpp
    src/grid.cpp
    src/simulation.cpp
    src/terrain.cpp
)

target_include_directories(floodsim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(floodsim_core PUBLIC cxx_std_20)

if(FLOODSIM_BUILD_TESTS)
    add_executable(floodsim_core_tests
        tests/test_simulation.cpp
    )

    target_link_libraries(floodsim_core_tests
        PRIVATE
            floodsim_core
    )

    target_compile_features(floodsim_core_tests PRIVATE cxx_std_20)

    add_test(NAME floodsim_core_tests COMMAND floodsim_core_tests)
endif()
