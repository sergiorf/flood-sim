#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "raster_frame.hpp"

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path write_csv(
    const std::filesystem::path& directory,
    const std::string& name,
    const std::string& scenario_name,
    const double water_depth) {
    const std::filesystem::path path = directory / name;
    std::ofstream output(path);
    output
        << "# rows,2\n"
        << "# cols,2\n"
        << "# cell_size_m,30\n"
        << "# scenario_name," << scenario_name << '\n'
        << "row,col,elevation_m,water_depth_m,surface_height_m\n"
        << "0,0,100," << water_depth << "," << (100.0 + water_depth) << '\n'
        << "0,1,101," << water_depth << "," << (101.0 + water_depth) << '\n'
        << "1,0,102," << water_depth << "," << (102.0 + water_depth) << '\n'
        << "1,1,103," << water_depth << "," << (103.0 + water_depth) << '\n';
    return path;
}

}  // namespace

TEST_CASE("native viewer discovers snapshot series from final csv") {
    const std::filesystem::path temp_dir =
        std::filesystem::temp_directory_path() / "floodsim_native_viewer_tests_series";
    std::filesystem::create_directories(temp_dir);

    const std::filesystem::path snapshot_one = write_csv(
        temp_dir,
        "brussels_demo_output_step0004_t1200s.csv",
        "baseline",
        0.01);
    const std::filesystem::path snapshot_two = write_csv(
        temp_dir,
        "brussels_demo_output_step0008_t2400s.csv",
        "baseline",
        0.02);
    const std::filesystem::path final_path = write_csv(
        temp_dir,
        "brussels_demo_output.csv",
        "baseline",
        0.03);

    const auto document = floodsim::native_viewer::load_raster_document(final_path);

    REQUIRE(document.frames.size() == 3);
    CHECK(document.frames[0].source_path == snapshot_one);
    CHECK(document.frames[1].source_path == snapshot_two);
    CHECK(document.frames[2].source_path == final_path);
    CHECK(document.frames[2].has_water_depth);

    std::filesystem::remove_all(temp_dir);
}

TEST_CASE("native viewer formats cell details for csv frames") {
    const std::filesystem::path temp_dir =
        std::filesystem::temp_directory_path() / "floodsim_native_viewer_tests_cells";
    std::filesystem::create_directories(temp_dir);

    const std::filesystem::path final_path = write_csv(
        temp_dir,
        "brussels_demo_output.csv",
        "baseline",
        0.03);

    const auto frame = floodsim::native_viewer::load_raster_frame(final_path);
    const std::string details = floodsim::native_viewer::format_cell_details(
        frame,
        floodsim::native_viewer::RasterLayer::WaterDepth,
        1,
        0);

    CHECK(details.find("row=1") != std::string::npos);
    CHECK(details.find("col=0") != std::string::npos);
    CHECK(details.find("layer=water_depth") != std::string::npos);
    CHECK(details.find("value=0.030") != std::string::npos);
    CHECK(details.find("elevation_m=102.000") != std::string::npos);

    std::filesystem::remove_all(temp_dir);
}
