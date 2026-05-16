#include "example_runner.hpp"

#include <cstddef>
#include <stdexcept>

namespace floodsim::examples::real_terrain {

const char* default_scenario_name() {
    return "baseline";
}

bool has_cli_scenario_overrides(const ScenarioOverrides& overrides) {
    return overrides.boundary_mode.has_value() ||
        overrides.rainfall_intensity_m_per_hour.has_value() ||
        overrides.rainfall_profile.has_value() ||
        overrides.runoff_coefficient.has_value() ||
        overrides.initial_loss_m.has_value() ||
        overrides.time_step_seconds.has_value() ||
        overrides.step_count.has_value();
}

const ScenarioPreset& find_scenario_preset(std::string_view name) {
    for (const ScenarioPreset& preset : scenario_presets()) {
        if (preset.name == name) {
            return preset;
        }
    }

    throw std::runtime_error(
        "Unknown scenario preset: " + std::string(name) +
        ". Expected one of: baseline, intense_short, long_moderate");
}

void apply_scenario_preset(ScenarioConfig& scenario, const ScenarioPreset& preset) {
    scenario.name = std::string(preset.name);
    scenario.rainfall_intensity_m_per_hour = preset.rainfall_intensity_m_per_hour;
    scenario.time_step_seconds = preset.time_step_seconds;
    scenario.step_count = preset.step_count;
    scenario.preset_applied = true;
}

void apply_scenario_overrides(ScenarioConfig& scenario, const ScenarioOverrides& overrides) {
    if (overrides.boundary_mode.has_value()) {
        scenario.boundary_mode = *overrides.boundary_mode;
    }
    if (overrides.rainfall_intensity_m_per_hour.has_value()) {
        scenario.rainfall_profile.reset();
        scenario.rainfall_intensity_m_per_hour = *overrides.rainfall_intensity_m_per_hour;
    }
    if (overrides.rainfall_profile.has_value()) {
        scenario.rainfall_intensity_m_per_hour = 0.0;
        scenario.rainfall_profile = overrides.rainfall_profile;
        scenario.step_count = static_cast<int>(scenario.rainfall_profile->step_intensities_m_per_hour.size());
    }
    if (overrides.runoff_coefficient.has_value()) {
        scenario.runoff_coefficient = *overrides.runoff_coefficient;
    }
    if (overrides.initial_loss_m.has_value()) {
        scenario.initial_loss_m = *overrides.initial_loss_m;
    }
    if (overrides.time_step_seconds.has_value()) {
        scenario.time_step_seconds = *overrides.time_step_seconds;
    }
    if (overrides.step_count.has_value()) {
        scenario.step_count = *overrides.step_count;
    }
}

void validate_scenario_config(const ScenarioConfig& scenario) {
    if (scenario.name.empty()) {
        throw std::runtime_error("Scenario name must not be empty");
    }
    if (scenario.rainfall_intensity_m_per_hour < 0.0) {
        throw std::runtime_error("Rainfall intensity must be non-negative");
    }
    if (scenario.rainfall_profile.has_value() && scenario.rainfall_intensity_m_per_hour > 0.0) {
        throw std::runtime_error("Scenario cannot define both uniform rainfall intensity and a rainfall profile");
    }
    if (!scenario.rainfall_profile.has_value() && scenario.rainfall_intensity_m_per_hour == 0.0) {
        throw std::runtime_error("Scenario must define either a rainfall intensity or a rainfall profile");
    }
    if (scenario.rainfall_profile.has_value() &&
        scenario.rainfall_profile->step_intensities_m_per_hour.empty()) {
        throw std::runtime_error("Rainfall profile must contain at least one step");
    }
    if (scenario.runoff_coefficient < 0.0 || scenario.runoff_coefficient > 1.0) {
        throw std::runtime_error("Runoff coefficient must be in [0, 1]");
    }
    if (scenario.initial_loss_m < 0.0) {
        throw std::runtime_error("Initial loss must be non-negative");
    }
    if (scenario.time_step_seconds <= 0.0) {
        throw std::runtime_error("Time step must be positive");
    }
    if (scenario.step_count <= 0) {
        throw std::runtime_error("Step count must be positive");
    }
    if (scenario.rainfall_profile.has_value() &&
        static_cast<std::size_t>(scenario.step_count) !=
            scenario.rainfall_profile->step_intensities_m_per_hour.size()) {
        throw std::runtime_error("Step count must match the rainfall profile length");
    }
}

}  // namespace floodsim::examples::real_terrain
