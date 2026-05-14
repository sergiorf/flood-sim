#include "example_runner.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        const auto arguments = floodsim::examples::real_terrain::parse_arguments(argc, argv);
        if (!arguments.batch_scenario_names.empty() || arguments.scenario_definitions.size() > 1) {
            bool had_failures = false;
            const auto batch_arguments =
                floodsim::examples::real_terrain::build_batch_scenario_arguments(arguments);
            std::vector<floodsim::examples::real_terrain::BatchScenarioResult> batch_results;
            batch_results.reserve(batch_arguments.size());

            for (const auto& scenario_arguments : batch_arguments) {
                try {
                    const auto result = floodsim::examples::real_terrain::run_example(scenario_arguments);
                    floodsim::examples::real_terrain::write_export(
                        result.grid,
                        result.loaded_terrain.terrain,
                        scenario_arguments.scenario,
                        scenario_arguments.scenario.output_csv_path);
                    floodsim::examples::real_terrain::write_snapshot_exports(
                        result,
                        scenario_arguments.scenario,
                        scenario_arguments.scenario.output_csv_path);
                    floodsim::examples::real_terrain::print_run_report(
                        std::cout,
                        scenario_arguments,
                        result);
                    for (const auto& snapshot : result.snapshots) {
                        std::cout << "wrote_snapshot_csv=\""
                                  << floodsim::examples::real_terrain::derive_snapshot_output_path(
                                         scenario_arguments.scenario.output_csv_path,
                                         snapshot.completed_steps,
                                         snapshot.elapsed_seconds)
                                         .string()
                                  << "\" completed_steps=" << snapshot.completed_steps
                                  << " elapsed_seconds=" << snapshot.elapsed_seconds << '\n';
                    }
                    batch_results.push_back(
                        floodsim::examples::real_terrain::BatchScenarioResult {
                            .arguments = scenario_arguments,
                            .result = result,
                        });
                } catch (const std::exception& error) {
                    had_failures = true;
                    std::cerr << "scenario_failed name=" << scenario_arguments.scenario.name
                              << " output_csv=\"" << scenario_arguments.scenario.output_csv_path.string()
                              << "\" error=\"" << error.what() << "\"\n";
                }
            }

            if (!batch_results.empty()) {
                const auto comparison_path =
                    floodsim::examples::real_terrain::derive_batch_comparison_output_path(
                        arguments.scenario.output_csv_path);
                floodsim::examples::real_terrain::write_batch_comparison_csv(
                    batch_results,
                    comparison_path);
                std::cout << "wrote_comparison_csv=\"" << comparison_path.string() << "\"\n";
            }

            return had_failures ? 1 : 0;
        }

        const auto result = floodsim::examples::real_terrain::run_example(arguments);
        floodsim::examples::real_terrain::write_export(
            result.grid,
            result.loaded_terrain.terrain,
            arguments.scenario,
            arguments.scenario.output_csv_path);
        floodsim::examples::real_terrain::write_snapshot_exports(
            result,
            arguments.scenario,
            arguments.scenario.output_csv_path);
        floodsim::examples::real_terrain::print_run_report(std::cout, arguments, result);
        for (const auto& snapshot : result.snapshots) {
            std::cout << "wrote_snapshot_csv=\""
                      << floodsim::examples::real_terrain::derive_snapshot_output_path(
                             arguments.scenario.output_csv_path,
                             snapshot.completed_steps,
                             snapshot.elapsed_seconds)
                             .string()
                      << "\" completed_steps=" << snapshot.completed_steps
                      << " elapsed_seconds=" << snapshot.elapsed_seconds << '\n';
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
