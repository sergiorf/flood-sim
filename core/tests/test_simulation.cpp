#include "floodsim/export.hpp"
#include "floodsim/grid.hpp"
#include "floodsim/simulation.hpp"
#include "floodsim/terrain.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using floodsim::BoundaryMode;
using floodsim::Grid;
using floodsim::RainfallScenario;
using floodsim::SimulationConfig;
using floodsim::TerrainRaster;

bool nearly_equal(double lhs, double rhs, double epsilon = 1e-9) {
    return std::fabs(lhs - rhs) <= epsilon;
}

void expect_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename ExceptionType, typename Function>
void expect_throws(Function&& function, const std::string& message) {
    try {
        function();
    } catch (const ExceptionType&) {
        return;
    } catch (...) {
        throw std::runtime_error(message + ": wrong exception type");
    }

    throw std::runtime_error(message + ": expected exception was not thrown");
}

void test_rainfall_adds_water() {
    Grid grid(2, 2);
    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 3600.0);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.012), "rainfall depth mismatch");
    expect_true(nearly_equal(grid.total_water_depth(), 0.048), "total rainfall water mismatch");
}

void test_rainfall_scales_with_step_duration() {
    Grid grid(1, 1);
    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 1800.0);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.006), "half-hour rainfall should add half of the hourly depth");
}

void test_water_flows_downhill() {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.5), "uphill cell should lose water");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.5), "downhill cell should gain water");
}

void test_rainfall_is_applied_before_flow() {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 0.0);

    RainfallScenario rainfall {1.0};
    SimulationConfig config {
        .time_step_seconds = 3600.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.5), "source cell should retain half of rainfall");
    expect_true(nearly_equal(grid.water_depth(0, 1), 1.5), "lower cell should receive rainfall plus routed inflow");
}

void test_outflow_is_split_by_relative_drop() {
    Grid grid(3, 3);
    grid.set_elevation(1, 1, 5.0);
    grid.set_elevation(0, 1, 3.0);
    grid.set_elevation(1, 0, 4.0);
    grid.set_elevation(1, 2, 7.0);
    grid.set_elevation(2, 1, 7.0);
    grid.set_water_depth(1, 1, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.6,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(1, 1), 0.4), "source cell should lose the configured outflow fraction");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.36), "steeper neighbor should receive three fifths of the outflow");
    expect_true(nearly_equal(grid.water_depth(1, 0), 0.24), "shallower neighbor should receive two fifths of the outflow");
}

void test_surface_height_includes_existing_water() {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 0.4);
    grid.set_water_depth(0, 1, 1.4);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.4), "equal surface heights should prevent flow despite different terrain");
    expect_true(nearly_equal(grid.water_depth(0, 1), 1.4), "existing neighbor water depth should participate in the surface-height comparison");
}

void test_flow_uses_a_full_grid_snapshot() {
    Grid grid(1, 3);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(0, 2, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.5), "left cell should lose half its water");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.5), "middle cell should receive inflow but not relay it in the same step");
    expect_true(nearly_equal(grid.water_depth(0, 2), 0.0), "right cell should remain dry until a later step");
}

void test_repeated_steps_relay_prior_inflow_on_later_steps() {
    Grid grid(1, 3);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(0, 2, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);
    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.25), "left cell should continue losing water over repeated steps");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.5), "middle cell should relay part of the prior step inflow on the next step");
    expect_true(nearly_equal(grid.water_depth(0, 2), 0.25), "right cell should only receive routed water on a later step");
}

void test_closed_boundary_keeps_corner_water_in_domain() {
    Grid grid(2, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(1, 0, 0.0);
    grid.set_elevation(1, 1, 3.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.5), "corner cell should only route water to in-domain lower neighbors");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.2), "edge neighbor should receive two fifths of the routed water");
    expect_true(nearly_equal(grid.water_depth(1, 0), 0.3), "lower in-domain neighbor should receive three fifths of the routed water");
    expect_true(nearly_equal(grid.total_water_depth(), 1.0), "closed boundary should not lose water off the grid");
}

void test_closed_boundary_can_be_selected_explicitly_in_config() {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
        .boundary_mode = BoundaryMode::Closed,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.5), "explicit closed boundary should preserve current default routing");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.5), "explicit closed boundary should route only to in-domain lower neighbors");
}

void test_closed_boundary_blocks_outflow_from_edge_when_no_lower_in_domain_neighbor_exists() {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 5.0);
    grid.set_elevation(0, 1, 6.0);
    grid.set_water_depth(0, 0, 0.8);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.75,
    };

    floodsim::step(grid, rainfall, config);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.8), "edge cell should retain water when the only in-domain neighbor is not lower");
    expect_true(nearly_equal(grid.water_depth(0, 1), 0.0), "higher edge neighbor should not receive flow");
    expect_true(nearly_equal(grid.total_water_depth(), 0.8), "closed boundary should preserve water when out-of-domain space is ignored");
}

void test_water_is_conserved_without_rainfall() {
    Grid grid(3, 3);
    grid.set_elevation(1, 1, 2.0);
    grid.set_elevation(1, 2, 1.0);
    grid.set_elevation(2, 2, 0.0);
    grid.set_water_depth(1, 1, 0.6);
    grid.set_water_depth(0, 0, 0.4);

    const double before = grid.total_water_depth();

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.25,
    };

    for (int i = 0; i < 8; ++i) {
        floodsim::step(grid, rainfall, config);
    }

    const double after = grid.total_water_depth();
    expect_true(nearly_equal(before, after, 1e-8), "water should remain conserved");
}

void test_repeated_steps_accumulate_rainfall_linearly_without_flow() {
    Grid grid(1, 1);
    RainfallScenario rainfall {0.008};
    SimulationConfig config {
        .time_step_seconds = 900.0,
        .max_outflow_fraction = 0.25,
    };

    for (int i = 0; i < 4; ++i) {
        floodsim::step(grid, rainfall, config);
    }

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.008), "four fifteen-minute steps should accumulate one hour of rainfall depth");
}

void test_csv_export_writes_metadata_header_and_per_cell_rows() {
    Grid grid(2, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 1.5);
    grid.set_elevation(1, 0, 2.0);
    grid.set_elevation(1, 1, 2.5);
    grid.set_water_depth(0, 0, 0.25);
    grid.set_water_depth(1, 1, 0.75);

    std::ostringstream output;
    floodsim::write_grid_csv(grid, output);

    const std::string expected =
        "# floodsim_csv_version,1\n"
        "# rows,2\n"
        "# cols,2\n"
        "# cell_size_m,1.000000\n"
        "row,col,elevation_m,water_depth_m,surface_height_m\n"
        "0,0,1.000000,0.250000,1.250000\n"
        "0,1,1.500000,0.000000,1.500000\n"
        "1,0,2.000000,0.000000,2.000000\n"
        "1,1,2.500000,0.750000,3.250000\n";

    expect_true(output.str() == expected, "CSV export should write stable metadata and row-major cell records");
}

TerrainRaster make_valid_terrain_raster() {
    TerrainRaster terrain;
    terrain.rows = 2;
    terrain.cols = 3;
    terrain.cell_size_m = 2.0;
    terrain.elevation_m = {
        101.2, 100.7, 100.1,
         99.9,   0.0,  98.8,
    };
    terrain.valid_cell_mask = {
        1, 1, 1,
        1, 0, 1,
    };
    terrain.origin_x_m = 154320.0;
    terrain.origin_y_m = 171205.0;
    terrain.crs_id = "EPSG:31370";
    return terrain;
}

void test_valid_terrain_raster_contract_passes_validation() {
    TerrainRaster terrain = make_valid_terrain_raster();

    floodsim::validate_terrain_raster(terrain);

    expect_true(terrain.cell_count() == 6, "terrain cell count should match rows * cols");
}

void test_terrain_raster_requires_matching_array_sizes() {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.valid_cell_mask.pop_back();

    expect_throws<std::invalid_argument>(
        [&terrain]() { floodsim::validate_terrain_raster(terrain); },
        "terrain raster should reject a mask length that does not match rows * cols");
}

void test_terrain_raster_requires_positive_cell_size() {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.cell_size_m = 0.0;

    expect_throws<std::invalid_argument>(
        [&terrain]() { floodsim::validate_terrain_raster(terrain); },
        "terrain raster should reject non-positive cell sizes");
}

void test_terrain_raster_requires_at_least_one_valid_cell() {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.valid_cell_mask = {
        0, 0, 0,
        0, 0, 0,
    };

    expect_throws<std::invalid_argument>(
        [&terrain]() { floodsim::validate_terrain_raster(terrain); },
        "terrain raster should reject a domain with no valid cells");
}

void test_terrain_raster_requires_complete_origin_metadata() {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.origin_y_m.reset();

    expect_throws<std::invalid_argument>(
        [&terrain]() { floodsim::validate_terrain_raster(terrain); },
        "terrain raster should reject origin metadata that provides only one coordinate");
}

}  // namespace

int main() {
    try {
        test_rainfall_adds_water();
        test_rainfall_scales_with_step_duration();
        test_water_flows_downhill();
        test_rainfall_is_applied_before_flow();
        test_outflow_is_split_by_relative_drop();
        test_surface_height_includes_existing_water();
        test_flow_uses_a_full_grid_snapshot();
        test_repeated_steps_relay_prior_inflow_on_later_steps();
        test_closed_boundary_keeps_corner_water_in_domain();
        test_closed_boundary_can_be_selected_explicitly_in_config();
        test_closed_boundary_blocks_outflow_from_edge_when_no_lower_in_domain_neighbor_exists();
        test_water_is_conserved_without_rainfall();
        test_repeated_steps_accumulate_rainfall_linearly_without_flow();
        test_csv_export_writes_metadata_header_and_per_cell_rows();
        test_valid_terrain_raster_contract_passes_validation();
        test_terrain_raster_requires_matching_array_sizes();
        test_terrain_raster_requires_positive_cell_size();
        test_terrain_raster_requires_at_least_one_valid_cell();
        test_terrain_raster_requires_complete_origin_metadata();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All FloodSim tests passed.\n";
    return 0;
}
