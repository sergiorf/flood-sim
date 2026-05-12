#include "example_runner.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        const auto arguments = floodsim::examples::real_terrain::parse_arguments(argc, argv);
        const auto result = floodsim::examples::real_terrain::run_example(arguments);
        floodsim::examples::real_terrain::write_export(
            result.grid,
            result.loaded_terrain.terrain,
            arguments.scenario,
            arguments.scenario.output_csv_path);
        floodsim::examples::real_terrain::print_run_report(std::cout, arguments, result);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
