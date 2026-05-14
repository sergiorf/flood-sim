#include "example_runner.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        const auto arguments = floodsim::examples::real_terrain::parse_arguments(argc, argv);
        if (!arguments.batch_scenario_names.empty()) {
            bool had_failures = false;
            const auto batch_arguments =
                floodsim::examples::real_terrain::build_batch_scenario_arguments(arguments);

            for (const auto& scenario_arguments : batch_arguments) {
                try {
                    const auto result = floodsim::examples::real_terrain::run_example(scenario_arguments);
                    floodsim::examples::real_terrain::write_export(
                        result.grid,
                        result.loaded_terrain.terrain,
                        scenario_arguments.scenario,
                        scenario_arguments.scenario.output_csv_path);
                    floodsim::examples::real_terrain::print_run_report(
                        std::cout,
                        scenario_arguments,
                        result);
                } catch (const std::exception& error) {
                    had_failures = true;
                    std::cerr << "scenario_failed name=" << scenario_arguments.scenario.name
                              << " output_csv=\"" << scenario_arguments.scenario.output_csv_path.string()
                              << "\" error=\"" << error.what() << "\"\n";
                }
            }

            return had_failures ? 1 : 0;
        }

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
