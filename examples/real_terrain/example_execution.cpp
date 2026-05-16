#include "example_runner.hpp"

#include <exception>

namespace floodsim::examples::real_terrain {

namespace {

void write_run_artifacts(
    const ExampleArguments& arguments,
    const ExampleRunResult& result,
    std::ostream& output) {
    write_export(
        result.grid,
        result.loaded_terrain.terrain,
        arguments.area_definition,
        arguments.surface_class_config,
        arguments.scenario,
        arguments.scenario.output_csv_path);
    write_snapshot_exports(
        result,
        arguments.area_definition,
        arguments.surface_class_config,
        arguments.scenario,
        arguments.scenario.output_csv_path);
    print_run_report(output, arguments, result);
    for (const auto& snapshot : result.snapshots) {
        output << "wrote_snapshot_csv=\""
               << derive_snapshot_output_path(
                      arguments.scenario.output_csv_path,
                      snapshot.completed_steps,
                      snapshot.elapsed_seconds)
                      .string()
               << "\" completed_steps=" << snapshot.completed_steps
               << " elapsed_seconds=" << snapshot.elapsed_seconds << '\n';
    }
}

}  // namespace

int execute_example_cli(
    const ExampleArguments& arguments,
    std::ostream& output,
    std::ostream& error) {
    if (!arguments.batch_scenario_names.empty() || arguments.scenario_definitions.size() > 1) {
        bool had_failures = false;
        const auto batch_arguments = build_batch_scenario_arguments(arguments);
        std::vector<BatchScenarioResult> batch_results;
        batch_results.reserve(batch_arguments.size());

        for (const auto& scenario_arguments : batch_arguments) {
            try {
                const auto result = run_example(scenario_arguments);
                write_run_artifacts(scenario_arguments, result, output);
                batch_results.push_back(
                    BatchScenarioResult {
                        .arguments = scenario_arguments,
                        .result = result,
                    });
            } catch (const std::exception& run_error) {
                had_failures = true;
                error << "scenario_failed name=" << scenario_arguments.scenario.name
                      << " output_csv=\"" << scenario_arguments.scenario.output_csv_path.string()
                      << "\" error=\"" << run_error.what() << "\"\n";
            }
        }

        if (!batch_results.empty()) {
            const auto comparison_path =
                derive_batch_comparison_output_path(arguments.scenario.output_csv_path);
            write_batch_comparison_csv(batch_results, comparison_path);
            output << "wrote_comparison_csv=\"" << comparison_path.string() << "\"\n";
        }

        return had_failures ? 1 : 0;
    }

    const auto result = run_example(arguments);
    write_run_artifacts(arguments, result, output);
    return 0;
}

}  // namespace floodsim::examples::real_terrain
