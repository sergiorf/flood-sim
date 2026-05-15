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
using floodsim::examples::real_terrain::build_batch_scenario_arguments;
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

void write_text_file(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path);
    REQUIRE(output.good());
    output << text;
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

TEST_CASE("real terrain helper loads single external scenario definition file") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--scenario-file",
            fixture_path("examples/real_terrain/data/sample_single_scenario.csv").string(),
        });

    CHECK(arguments.scenario.file_applied);
    CHECK(arguments.scenario.name == "reviewed_screening");
    CHECK(arguments.scenario.rainfall_intensity_m_per_hour == doctest::Approx(0.012));
    CHECK(arguments.scenario.runoff_coefficient == doctest::Approx(1.0));
    CHECK(arguments.scenario.initial_loss_m == doctest::Approx(0.0));
    CHECK(arguments.scenario.time_step_seconds == doctest::Approx(300.0));
    CHECK(arguments.scenario.step_count == 12);
    CHECK(arguments.scenario.boundary_mode == floodsim::BoundaryMode::Open);
    CHECK(scenario_source_to_string(arguments.scenario) == "file");
}

TEST_CASE("real terrain helper loads batch external scenario definition file") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "file_batch.csv",
            "--scenario-file",
            fixture_path("examples/real_terrain/data/sample_scenarios.csv").string(),
        });

    REQUIRE(arguments.scenario_definitions.size() == 3);
    CHECK(arguments.scenario_definitions[0].name == "baseline_file");
    CHECK(arguments.scenario_definitions[1].name == "intense_short_file");
    CHECK(arguments.scenario_definitions[2].name == "long_moderate_file");

    const auto batch_arguments = build_batch_scenario_arguments(arguments);
    REQUIRE(batch_arguments.size() == 3);
    CHECK(batch_arguments[0].scenario.output_csv_path == std::filesystem::path("file_batch_baseline_file.csv"));
    CHECK(batch_arguments[1].scenario.output_csv_path == std::filesystem::path("file_batch_intense_short_file.csv"));
    CHECK(batch_arguments[2].scenario.output_csv_path == std::filesystem::path("file_batch_long_moderate_file.csv"));
}

TEST_CASE("real terrain helper loads area definition file with provenance and clipped window") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            "--area-file",
            fixture_path("examples/real_terrain/data/sample_area_clip.csv").string(),
            "output.csv",
            "--scenario",
            "baseline",
        });

    REQUIRE(arguments.area_definition.has_value());
    CHECK(arguments.area_definition->area_name == "sample_center_clip");
    CHECK(arguments.area_definition->input_dem_path == fixture_path("examples/real_terrain/data/sample_dem.tif"));
    CHECK(arguments.area_definition->source_name == "checked_in_sample_dem");
    CHECK(arguments.area_definition->source_details == "checked_in_demo_clip");
    CHECK(!arguments.area_definition->boundary_path.has_value());
    REQUIRE(arguments.terrain_window.has_value());
    CHECK(arguments.terrain_window->row_offset == 1);
    CHECK(arguments.terrain_window->col_offset == 1);
    CHECK(arguments.terrain_window->rows == 3);
    CHECK(arguments.terrain_window->cols == 2);
}

TEST_CASE("real terrain helper parses batch scenario list") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "batch.csv",
            "--batch-scenarios",
            "baseline,intense_short,long_moderate",
            "--runoff-coefficient",
            "0.5",
        });

    CHECK(arguments.batch_scenario_names.size() == 3);
    CHECK(arguments.batch_scenario_names[0] == "baseline");
    CHECK(arguments.batch_scenario_names[1] == "intense_short");
    CHECK(arguments.batch_scenario_names[2] == "long_moderate");
    CHECK(arguments.scenario_overrides.runoff_coefficient == doctest::Approx(0.5));
}

TEST_CASE("real terrain helper parses snapshot interval") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--snapshot-every-steps",
            "4",
        });

    REQUIRE(arguments.snapshot_every_steps.has_value());
    CHECK(*arguments.snapshot_every_steps == 4);
}

TEST_CASE("real terrain helper loads rainfall profile and derives step count") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--rainfall-profile-file",
            fixture_path("examples/real_terrain/data/sample_storm_profile.csv").string(),
        });

    REQUIRE(arguments.scenario.rainfall_profile.has_value());
    CHECK(arguments.scenario.rainfall_profile->source_path ==
          fixture_path("examples/real_terrain/data/sample_storm_profile.csv"));
    REQUIRE(arguments.scenario.rainfall_profile->step_intensities_m_per_hour.size() == 6);
    CHECK(arguments.scenario.rainfall_profile->step_intensities_m_per_hour[0] == doctest::Approx(0.0));
    CHECK(arguments.scenario.rainfall_profile->step_intensities_m_per_hour[3] == doctest::Approx(0.03));
    CHECK(arguments.scenario.step_count == 6);
}

TEST_CASE("real terrain helper expands batch scenarios into deterministic output paths") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "batch_outputs.csv",
            "--batch-scenarios",
            "baseline,intense_short",
            "--boundary-mode",
            "closed",
        });

    const auto batch_arguments = build_batch_scenario_arguments(arguments);

    REQUIRE(batch_arguments.size() == 2);
    CHECK(batch_arguments[0].scenario.name == "baseline");
    CHECK(batch_arguments[0].scenario.output_csv_path == std::filesystem::path("batch_outputs_baseline.csv"));
    CHECK(batch_arguments[0].scenario.boundary_mode == floodsim::BoundaryMode::Closed);
    CHECK(batch_arguments[1].scenario.name == "intense_short");
    CHECK(batch_arguments[1].scenario.output_csv_path == std::filesystem::path("batch_outputs_intense_short.csv"));
    CHECK(batch_arguments[1].scenario.boundary_mode == floodsim::BoundaryMode::Closed);
}

TEST_CASE("real terrain helper derives deterministic batch comparison path") {
    const auto comparison_path =
        floodsim::examples::real_terrain::derive_batch_comparison_output_path("batch_outputs.csv");
    CHECK(comparison_path == std::filesystem::path("batch_outputs_comparison.csv"));
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
                "--area-file",
                fixture_path("examples/real_terrain/data/invalid_area_missing_provenance.csv").string(),
                "output.csv",
            })),
        doctest::Contains("Area definition must provide a non-empty source_name"));

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

    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--scenario",
                "baseline",
                "--batch-scenarios",
                "intense_short,long_moderate",
            })),
        doctest::Contains("Use only one of --scenario, --batch-scenarios, or --scenario-file"));

    const auto invalid_header_path = std::filesystem::temp_directory_path() / "floodsim_invalid_scenario_header.csv";
    write_text_file(
        invalid_header_path,
        "scenario,bad_header\n"
        "demo,0.012\n");
    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--scenario-file",
                invalid_header_path.string(),
            })),
        doctest::Contains("Scenario file has invalid header"));
    std::filesystem::remove(invalid_header_path);

    const auto invalid_boundary_path = std::filesystem::temp_directory_path() / "floodsim_invalid_scenario_boundary.csv";
    write_text_file(
        invalid_boundary_path,
        "scenario_name,rainfall_intensity_m_per_hour,runoff_coefficient,initial_loss_m,time_step_seconds,steps,boundary_mode\n"
        "demo,0.012000,1.000000,0.000000,300.000000,12,sideways\n");
    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--scenario-file",
                invalid_boundary_path.string(),
            })),
        doctest::Contains("Invalid value for scenario file boundary mode"));
    std::filesystem::remove(invalid_boundary_path);

    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--snapshot-every-steps",
                "0",
            })),
        doctest::Contains("Snapshot interval must be positive"));

    CHECK_THROWS_WITH(
        static_cast<void>(parse_arguments(
            {
                "floodsim_real_terrain_example",
                fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
                "output.csv",
                "--rainfall-profile-file",
                fixture_path("examples/real_terrain/data/sample_storm_profile.csv").string(),
                "--steps",
                "12",
            })),
        doctest::Contains("Do not use --steps with --rainfall-profile-file"));
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
    CHECK(report.str().find("rainfall_mode=uniform") != std::string::npos);
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
    write_export(result.grid, result.loaded_terrain.terrain, arguments.area_definition, arguments.scenario, export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# rows,4") != std::string::npos);
    CHECK(export_text.find("# cols,4") != std::string::npos);
    CHECK(export_text.find("# origin_x_m,5000.000000") != std::string::npos);
    CHECK(export_text.find("# origin_y_m,1020.000000") != std::string::npos);
    CHECK(export_text.find("# scenario_name,baseline") != std::string::npos);
    CHECK(export_text.find("# boundary_mode,open") != std::string::npos);
    CHECK(export_text.find("# rainfall_mode,uniform") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("real terrain helper captures deterministic intermediate snapshots") {
    const auto arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/sample_dem.tif").string(),
            "output.csv",
            "--snapshot-every-steps",
            "4",
        });
    const auto result = run_example(arguments);

    REQUIRE(result.snapshots.size() == 2);
    CHECK(result.snapshots[0].completed_steps == 4);
    CHECK(result.snapshots[0].elapsed_seconds == doctest::Approx(1200.0));
    CHECK(result.snapshots[1].completed_steps == 8);
    CHECK(result.snapshots[1].elapsed_seconds == doctest::Approx(2400.0));
    CHECK(result.snapshots[0].summary_metrics.wet_cell_count == 24);
    CHECK(result.snapshots[1].summary_metrics.wet_cell_count == 24);

    const auto first_snapshot_path =
        floodsim::examples::real_terrain::derive_snapshot_output_path("output.csv", 4, 1200.0);
    const auto second_snapshot_path =
        floodsim::examples::real_terrain::derive_snapshot_output_path("output.csv", 8, 2400.0);
    CHECK(first_snapshot_path == std::filesystem::path("output_step0004_t1200s.csv"));
    CHECK(second_snapshot_path == std::filesystem::path("output_step0008_t2400s.csv"));
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
        reduced_runoff_arguments.area_definition,
        reduced_runoff_arguments.scenario,
        export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# runoff_coefficient,0.500000") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("real terrain helper initial loss delays runoff and is exported") {
    ExampleArguments baseline_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "baseline.csv",
        });
    const auto baseline_result = run_example(baseline_arguments);

    ExampleArguments initial_loss_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "initial_loss.csv",
            "--initial-loss-m",
            "0.002",
        });
    const auto initial_loss_result = run_example(initial_loss_arguments);

    CHECK(initial_loss_result.grid.total_water_depth() < baseline_result.grid.total_water_depth());
    CHECK(initial_loss_result.summary_metrics.max_water_depth_m < baseline_result.summary_metrics.max_water_depth_m);

    std::ostringstream report;
    print_run_report(report, initial_loss_arguments, initial_loss_result);
    CHECK(report.str().find("initial_loss_m=0.002000") != std::string::npos);

    const auto export_path = std::filesystem::temp_directory_path() / "floodsim_real_terrain_initial_loss_export.csv";
    write_export(
        initial_loss_result.grid,
        initial_loss_result.loaded_terrain.terrain,
        initial_loss_arguments.area_definition,
        initial_loss_arguments.scenario,
        export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# initial_loss_m,0.002000") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("real terrain helper supports external rainfall profile events") {
    ExampleArguments baseline_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "baseline.csv",
        });
    const auto baseline_result = run_example(baseline_arguments);

    ExampleArguments profile_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/drainage_slope.asc").string(),
            "profile.csv",
            "--rainfall-profile-file",
            fixture_path("examples/real_terrain/data/sample_storm_profile.csv").string(),
        });
    const auto profile_result = run_example(profile_arguments);

    REQUIRE(profile_arguments.scenario.rainfall_profile.has_value());
    CHECK(profile_arguments.scenario.step_count == 6);
    CHECK(profile_result.grid.total_water_depth() < baseline_result.grid.total_water_depth());

    std::ostringstream report;
    print_run_report(report, profile_arguments, profile_result);
    CHECK(report.str().find("rainfall_mode=profile") != std::string::npos);
    CHECK(report.str().find("rainfall_profile_path=") != std::string::npos);
    CHECK(report.str().find("peak_rainfall_intensity_m_per_hour=0.030000") != std::string::npos);
    CHECK(report.str().find("total_rainfall_depth_m=0.007500") != std::string::npos);

    const auto export_path = std::filesystem::temp_directory_path() / "floodsim_real_terrain_profile_export.csv";
    write_export(
        profile_result.grid,
        profile_result.loaded_terrain.terrain,
        profile_arguments.area_definition,
        profile_arguments.scenario,
        export_path);
    const std::string export_text = slurp_file(export_path);
    CHECK(export_text.find("# rainfall_mode,profile") != std::string::npos);
    CHECK(export_text.find("# rainfall_profile_path,") != std::string::npos);
    CHECK(export_text.find("# peak_rainfall_intensity_m_per_hour,0.030000") != std::string::npos);
    CHECK(export_text.find("# total_rainfall_depth_m,0.007500") != std::string::npos);
    std::filesystem::remove(export_path);
}

TEST_CASE("named real-terrain scenarios produce deterministic comparison metrics on sample clip") {
    const auto sample_dem = fixture_path("examples/real_terrain/data/sample_dem.tif").string();

    const ExampleArguments baseline_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            sample_dem,
            "baseline.csv",
        });
    const auto baseline_result = run_example(baseline_arguments);

    const ExampleArguments intense_short_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            sample_dem,
            "intense_short.csv",
            "--scenario",
            "intense_short",
        });
    const auto intense_short_result = run_example(intense_short_arguments);

    const ExampleArguments long_moderate_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            sample_dem,
            "long_moderate.csv",
            "--scenario",
            "long_moderate",
        });
    const auto long_moderate_result = run_example(long_moderate_arguments);

    CHECK(baseline_arguments.scenario.name == "baseline");
    CHECK(intense_short_arguments.scenario.name == "intense_short");
    CHECK(long_moderate_arguments.scenario.name == "long_moderate");

    CHECK(baseline_result.summary_metrics.wet_cell_count == 24);
    CHECK(intense_short_result.summary_metrics.wet_cell_count == 24);
    CHECK(long_moderate_result.summary_metrics.wet_cell_count == 24);

    CHECK(baseline_result.grid.total_water_depth() == doctest::Approx(0.287743).epsilon(1e-6));
    CHECK(intense_short_result.grid.total_water_depth() == doctest::Approx(0.359635).epsilon(1e-6));
    CHECK(long_moderate_result.grid.total_water_depth() == doctest::Approx(0.575294).epsilon(1e-6));

    CHECK(baseline_result.summary_metrics.max_water_depth_m == doctest::Approx(0.096997).epsilon(1e-6));
    CHECK(intense_short_result.summary_metrics.max_water_depth_m == doctest::Approx(0.067712).epsilon(1e-6));
    CHECK(long_moderate_result.summary_metrics.max_water_depth_m == doctest::Approx(0.402130).epsilon(1e-6));
}

TEST_CASE("additional real-terrain fixtures cover distinct retention patterns") {
    const auto flat_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/flat_pond.asc").string(),
            "flat.csv",
        });
    const auto flat_result = run_example(flat_arguments);

    const auto edge_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/edge_notch.asc").string(),
            "edge.csv",
        });
    const auto edge_result = run_example(edge_arguments);

    const auto urban_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/urban_block.asc").string(),
            "urban.csv",
        });
    const auto urban_result = run_example(urban_arguments);

    const auto split_arguments = parse_arguments(
        {
            "floodsim_real_terrain_example",
            fixture_path("examples/real_terrain/data/split_basin.asc").string(),
            "split.csv",
        });
    const auto split_result = run_example(split_arguments);

    CHECK(flat_result.grid.total_water_depth() == doctest::Approx(0.266520).epsilon(1e-6));
    CHECK(edge_result.grid.total_water_depth() == doctest::Approx(0.235312).epsilon(1e-6));
    CHECK(urban_result.grid.total_water_depth() == doctest::Approx(0.403640).epsilon(1e-6));
    CHECK(split_result.grid.total_water_depth() == doctest::Approx(0.419473).epsilon(1e-6));

    CHECK(flat_result.summary_metrics.max_water_depth_m == doctest::Approx(0.088587).epsilon(1e-6));
    CHECK(edge_result.summary_metrics.max_water_depth_m == doctest::Approx(0.024176).epsilon(1e-6));
    CHECK(urban_result.summary_metrics.max_water_depth_m == doctest::Approx(0.039982).epsilon(1e-6));
    CHECK(split_result.summary_metrics.max_water_depth_m == doctest::Approx(0.082292).epsilon(1e-6));

    CHECK(flat_result.summary_metrics.deepest_row == 2);
    CHECK(flat_result.summary_metrics.deepest_col == 2);
    CHECK(edge_result.summary_metrics.deepest_row == 0);
    CHECK(edge_result.summary_metrics.deepest_col == 3);
    CHECK(urban_result.summary_metrics.deepest_row == 2);
    CHECK(urban_result.summary_metrics.deepest_col == 2);
    CHECK(split_result.summary_metrics.deepest_row == 2);
    CHECK(split_result.summary_metrics.deepest_col == 2);
}

TEST_CASE("batch comparison export writes deterministic scenario summary table") {
    const auto sample_dem = fixture_path("examples/real_terrain/data/sample_dem.tif").string();
    const auto batch_arguments = build_batch_scenario_arguments(parse_arguments(
        {
            "floodsim_real_terrain_example",
            sample_dem,
            "batch_outputs.csv",
            "--batch-scenarios",
            "baseline,intense_short",
        }));

    std::vector<floodsim::examples::real_terrain::BatchScenarioResult> batch_results;
    for (const auto& arguments : batch_arguments) {
        batch_results.push_back(
            floodsim::examples::real_terrain::BatchScenarioResult {
                .arguments = arguments,
                .result = run_example(arguments),
            });
    }

    const auto export_path = std::filesystem::temp_directory_path() / "floodsim_real_terrain_batch_comparison.csv";
    floodsim::examples::real_terrain::write_batch_comparison_csv(batch_results, export_path);
    const std::string export_text = slurp_file(export_path);

    CHECK(export_text.find(
              "scenario_name,boundary_mode,rainfall_mode,rainfall_intensity_m_per_hour,rainfall_profile_path,"
              "peak_rainfall_intensity_m_per_hour,total_rainfall_depth_m,runoff_coefficient,initial_loss_m,"
              "time_step_seconds,steps,total_water_depth_m,wet_cells,max_water_depth_m,"
              "deepest_row,deepest_col,output_csv") == 0);
    CHECK(export_text.find("baseline,open,uniform,0.012000,,0.012000,0.012000,1.000000,0.000000,300.000000,12,0.287743,24,0.096997,2,2,batch_outputs_baseline.csv") != std::string::npos);
    CHECK(export_text.find("intense_short,open,uniform,0.030000,,0.030000,0.015000,1.000000,0.000000,300.000000,6,0.359635,24,0.067712,2,2,batch_outputs_intense_short.csv") != std::string::npos);
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
