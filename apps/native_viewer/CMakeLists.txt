find_package(SDL2 QUIET CONFIG)
if(NOT TARGET SDL2::SDL2)
    include(FindPkgConfig)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
endif()

add_library(floodsim_native_viewer_support
    raster_frame.cpp
)

target_link_libraries(floodsim_native_viewer_support
    PUBLIC
        floodsim_core
)

target_include_directories(floodsim_native_viewer_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(floodsim_native_viewer_support PUBLIC cxx_std_20)

add_executable(floodsim_native_viewer
    main.cpp
    sdl_graphics_backend.cpp
)

target_link_libraries(floodsim_native_viewer
    PRIVATE
        floodsim_native_viewer_support
)

if(TARGET SDL2::SDL2)
    target_link_libraries(floodsim_native_viewer PRIVATE SDL2::SDL2)
else()
    target_link_libraries(floodsim_native_viewer PRIVATE PkgConfig::SDL2)
endif()

target_compile_features(floodsim_native_viewer PRIVATE cxx_std_20)

if(FLOODSIM_BUILD_TESTS)
    add_executable(floodsim_native_viewer_tests
        test_raster_frame.cpp
    )

    target_link_libraries(floodsim_native_viewer_tests
        PRIVATE
            floodsim_native_viewer_support
            doctest::doctest
    )

    target_compile_features(floodsim_native_viewer_tests PRIVATE cxx_std_20)
    add_test(NAME floodsim_native_viewer_tests COMMAND floodsim_native_viewer_tests)
endif()
