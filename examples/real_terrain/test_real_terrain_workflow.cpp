#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "example_runner.hpp"

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using floodsim::examples::real_terrain::ExampleArguments;
using floodsim::examples::real_terrain::ExampleRunResult;
using floodsim::examples::real_terrain::parse_arguments;
using floodsim::examples::real_terrain::print_run_report;
using floodsim::examples::real_terrain::run_example;
using floodsim::examples::real_terrain::scenario_source_to_string;
using floodsim::examples::real_terrain::write_export;

std::filesystem::path fixture_path(const std::string& relative_path) {
    return std::filesystem::path(FLOODSIM_SOURCE_DIR) / relative_path;
}

std::string slurp_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    REQUIRE(input.good());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

ExampleRunResult run_fixture(const std::vector<std::string>& args, ExampleArguments& parsed_arguments) {
    parsed_arguments = parse_arguments(args);
    return run_example(parsed_arguments);
}

}  // namespace

TEST_CASE("real terrain helper parses preset overrides and clipped window") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--scenario",
            "long_moderate",
            "--rainfall-intensity-m-per-hour",
            "0.018",
            "--steps",
            "10",
            "--boundary-mode",
            "open",
            "--window-row-offset",
            "1",
            "--window-col-offset",
            "1",
            "--window-rows",
            "3",
            "--window-cols",
            "2",
        });

    CHECK(arguments.scenario.name == "long_moderate");
    CHECK(arguments.scenario.preset_applied);
    CHECK(arguments.scenario.cli_overrides_applied);
    CHECK(arguments.scenario.rainfall_intensity_m_per_hour == doctest::Approx(0.018));
    CHECK(arguments.scenario.time_step_seconds == doctest::Approx(300.0));
    CHECK(arguments.scenario.step_count == 10);
    CHECK(arguments.scenario.boundary_mode == floodsim::BoundaryMode::Open);
    REQUIRE(arguments.terrain_window.has_value());
    CHECK(arguments.terrain_window->row_offset == 1);
    CHECK(arguments.terrain_window->col_offset == 1);
    CHECK(arguments.terrain_window->rows == 3);
    CHECK(arguments.terrain_window->cols == 2);
    CHECK(scenario_source_to_string(arguments.scenario) == "preset_with_cli_overrides");
}

TEST_CASE("real terrain helper treats runoff coefficient as a CLI override") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--scenario",
            "baseline",
            "--runoff-coefficient",
            "0.5",
        });

    CHECK(arguments.scenario.name == "baseline");
    CHECK(arguments.scenario.preset_applied);
    CHECK(arguments.scenario.cli_overrides_applied);
    CHECK(arguments.scenario.runoff_coefficient == doctest::Approx(0.5));
    CHECK(scenario_source_to_string(arguments.scenario) == "preset_with_cli_overrides");
}

TEST_CASE("real terrain helper defaults to open boundary for clipped-terrain runs") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "output.csv",
        });

    CHECK(arguments.scenario.name == "baseline");
    CHECK(arguments.scenario.boundary_mode == floodsim::BoundaryMode::Open);
}

TEST_CASE("real terrain helper rejects invalid scenario configuration") {
    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--runoff-coefficient",
                "1.5",
            })),
        doctest::Contains("Runoff coefficient must be in [0, 1]"));

    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--window-row-offset",
                "1",
                "--window-cols",
                "2",
            })),
        doctest::Contains("Terrain window requires both --window-rows and --window-cols"));
}

TEST_CASE("real terrain helper runs nodata basin fixture and reports deterministic summary") {
    ExampleArguments arguments;
    const auto result = run_fixture(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
        },
        arguments);

    CHECK(result.loaded_terrain.terrain.rows == 5);
    CHECK(result.loaded_terrain.terrain.cols == 5);
    CHECK(result.loaded_terrain.report.invalid_cell_count == 1);
    CHECK(result.summary_metrics.wet_cell_count == 24);
    CHECK(result.summary_metrics.max_water_depth_m == doctest::Approx(0.096997).epsilon(1e-6));
    REQUIRE(result.summary_metrics.deepest_row.has_value());
    REQUIRE(result.summary_metrics.deepest_col.has_value());
    CHECK(*result.summary_metrics.deepest_row == 2);
    CHECK(*result.summary_metrics.deepest_col == 2);

    std::ostringstream report;
    print_run_report(report, arguments, result);
    CHECK(report.str().find("boundary_mode=open") != std::string::npos);
    CHECK(report.str().find("nodata_status=band_metadata_applied") != std::string::npos);
    CHECK(report.str().find("summary_metrics wet_cells=24 max_water_depth_m=0.096997 deepest_row=2 deepest_col=2") != std::string::npos);
}

TEST_CASE("real terrain helper runs drainage slope fixture and exports metadata") {
    ExampleArguments arguments;
    const auto result = run_fixture(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "output.csv",
        },
        arguments);

    CHECK(result.loaded_terrain.terrain.rows == 4);
    CHECK(result.loaded_terrain.terrain.cols == 4);
    CHECK(result.loaded_terrain.report.invalid_cell_count == 0);
    REQUIRE(result.summary_metrics.deepest_row.has_value());
    REQUIRE(result.summary_metrics.deepest_col.has_value());
    CHECK(*result.summary_metrics.deepest_row == 3);
    CHECK(*result.summary_metrics.deepest_col == 3);

    const auto export_path = std::filesystem::temp_directory_path() / "floodsim_real_terrain_helper_export.csv";
    write_export(result.grid, result.loaded_terrain.terrain, arguments.scenario, export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# rows,4") != std::string::npos);
    CHECK(export_text.find("# cols,4") != std::string::npos);
    CHECK(export_text.find("# origin_x_m,5000.000000") != std::string::npos);
    CHECK(export_text.find("# origin_y_m,1020.000000") != std::string::npos);
    CHECK(export_text.find("# scenario_name,baseline") != std::string::npos);
    CHECK(export_text.find("# boundary_mode,open") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("real terrain helper runoff coefficient reduces retained water and is exported") {
    ExampleArguments baseline_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "baseline.csv",
        });
    const auto baseline_result = run_example(baseline_arguments);

    ExampleArguments reduced_runoff_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "reduced.csv",
            "--runoff-coefficient",
            "0.5",
        });
    const auto reduced_runoff_result = run_example(reduced_runoff_arguments);

    CHECK(reduced_runoff_result.grid.total_water_depth() < baseline_result.grid.total_water_depth());
    CHECK(reduced_runoff_result.summary_metrics.max_water_depth_m < baseline_result.summary_metrics.max_water_depth_m);
    CHECK(reduced_runoff_result.summary_metrics.max_water_depth_m >= 0.0);

    std::ostringstream report;
    print_run_report(report, reduced_runoff_arguments, reduced_runoff_result);
    CHECK(report.str().find("runoff_coefficient=0.500000") != std::string::npos);

    const auto export_path = std::filesystem::temp_directory_path() / "floodsim_real_terrain_runoff_export.csv";
    write_export(
        reduced_runoff_result.grid,
        reduced_runoff_result.loaded_terrain.terrain,
        reduced_runoff_arguments.scenario,
        export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# runoff_coefficient,0.500000") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("open boundary retains less water than closed boundary on drainage slope fixture") {
    ExampleArguments open_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "open.csv",
        });
    const auto open_result = run_example(open_arguments);

    ExampleArguments closed_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "closed.csv",
            "--boundary-mode",
            "closed",
        });
    const auto closed_result = run_example(closed_arguments);

    CHECK(open_arguments.scenario.boundary_mode == floodsim::BoundaryMode::Open);
    CHECK(closed_arguments.scenario.boundary_mode == floodsim::BoundaryMode::Closed);
    CHECK(open_result.grid.total_water_depth() < closed_result.grid.total_water_depth());
    CHECK(open_result.summary_metrics.max_water_depth_m < closed_result.summary_metrics.max_water_depth_m);
}
