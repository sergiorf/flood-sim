#include "floodsim/grid.hpp"
#include "floodsim/simulation.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using floodsim::Grid;
using floodsim::RainfallScenario;
using floodsim::SimulationConfig;

bool nearly_equal(double lhs, double rhs, double epsilon = 1e-9) {
    return std::fabs(lhs - rhs) <= epsilon;
}

void expect_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_rainfall_adds_water() {
    Grid grid(2, 2);
    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 3600.0);

    expect_true(nearly_equal(grid.water_depth(0, 0), 0.012), "rainfall depth mismatch");
    expect_true(nearly_equal(grid.total_water_depth(), 0.048), "total rainfall water mismatch");
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

}  // namespace

int main() {
    try {
        test_rainfall_adds_water();
        test_water_flows_downhill();
        test_water_is_conserved_without_rainfall();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All FloodSim tests passed.\n";
    return 0;
}

