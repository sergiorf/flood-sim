#include "raster_frame.hpp"

#include "floodsim/terrain.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace floodsim::native_viewer {

namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream line_stream(line);
    std::string field;
    while (std::getline(line_stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

std::vector<double> dataset_for_layer(const RasterFrame& frame, const RasterLayer layer) {
    switch (layer) {
        case RasterLayer::Elevation:
            return frame.elevation;
        case RasterLayer::WaterDepth:
            return frame.water_depth;
        case RasterLayer::SurfaceHeight:
            return frame.surface_height;
    }

    throw std::runtime_error("Unhandled raster layer");
}

std::pair<double, double> finite_range(const std::vector<double>& values) {
    double min_value = std::numeric_limits<double>::infinity();
    double max_value = -std::numeric_limits<double>::infinity();
    for (const double value : values) {
        if (std::isnan(value)) {
            continue;
        }
        min_value = std::min(min_value, value);
        max_value = std::max(max_value, value);
    }
    if (!std::isfinite(min_value) || !std::isfinite(max_value)) {
        return {0.0, 0.0};
    }
    return {min_value, max_value};
}

std::uint32_t pack_rgba8(
    const std::uint8_t red,
    const std::uint8_t green,
    const std::uint8_t blue,
    const std::uint8_t alpha = 255) {
    return (static_cast<std::uint32_t>(red) << 24) |
        (static_cast<std::uint32_t>(green) << 16) |
        (static_cast<std::uint32_t>(blue) << 8) |
        static_cast<std::uint32_t>(alpha);
}

std::uint32_t terrain_color(const double normalized_value) {
    const double clamped = std::clamp(normalized_value, 0.0, 1.0);
    const std::uint8_t red = static_cast<std::uint8_t>(40.0 + clamped * 180.0);
    const std::uint8_t green = static_cast<std::uint8_t>(50.0 + clamped * 150.0);
    const std::uint8_t blue = static_cast<std::uint8_t>(30.0 + clamped * 90.0);
    return pack_rgba8(red, green, blue);
}

std::uint32_t water_color(const double normalized_value) {
    const double clamped = std::clamp(normalized_value, 0.0, 1.0);
    const std::uint8_t red = static_cast<std::uint8_t>(10.0 + clamped * 20.0);
    const std::uint8_t green = static_cast<std::uint8_t>(50.0 + clamped * 80.0);
    const std::uint8_t blue = static_cast<std::uint8_t>(110.0 + clamped * 130.0);
    return pack_rgba8(red, green, blue);
}

std::uint32_t surface_color(const double normalized_value) {
    const double clamped = std::clamp(normalized_value, 0.0, 1.0);
    const std::uint8_t red = static_cast<std::uint8_t>(50.0 + clamped * 140.0);
    const std::uint8_t green = static_cast<std::uint8_t>(50.0 + clamped * 110.0);
    const std::uint8_t blue = static_cast<std::uint8_t>(70.0 + clamped * 120.0);
    return pack_rgba8(red, green, blue);
}

std::uint32_t color_for_layer_value(
    const RasterLayer layer,
    const double value,
    const double min_value,
    const double max_value) {
    if (std::isnan(value)) {
        return pack_rgba8(24, 24, 24);
    }

    const double normalized = max_value <= min_value
        ? 0.0
        : (value - min_value) / (max_value - min_value);

    switch (layer) {
        case RasterLayer::Elevation:
            return terrain_color(normalized);
        case RasterLayer::WaterDepth:
            return water_color(normalized);
        case RasterLayer::SurfaceHeight:
            return surface_color(normalized);
    }

    throw std::runtime_error("Unhandled raster layer");
}

RasterFrame load_floodsim_csv_frame(const std::filesystem::path& input_path) {
    std::ifstream input(input_path);
    if (!input) {
        throw std::runtime_error("Failed to open FloodSim CSV: " + input_path.string());
    }

    RasterFrame frame {
        .source_path = input_path,
        .source_format = "floodsim_csv",
    };

    std::string line;
    bool header_seen = false;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        if (line.rfind("# ", 0) == 0) {
            const auto fields = split_csv_line(line.substr(2));
            if (fields.size() != 2) {
                throw std::runtime_error("Malformed FloodSim metadata line in " + input_path.string());
            }
            const std::string& key = fields[0];
            const std::string& value = fields[1];
            if (key == "rows") {
                frame.rows = static_cast<std::size_t>(std::stoull(value));
            } else if (key == "cols") {
                frame.cols = static_cast<std::size_t>(std::stoull(value));
            } else if (key == "cell_size_m") {
                frame.cell_size_m = std::stod(value);
            } else if (key == "origin_x_m") {
                frame.origin_x_m = std::stod(value);
            } else if (key == "origin_y_m") {
                frame.origin_y_m = std::stod(value);
            } else if (key == "crs_id") {
                frame.crs_id = value;
            } else if (key == "scenario_name") {
                frame.scenario_name = value;
            }
            continue;
        }

        if (!header_seen) {
            if (line != "row,col,elevation_m,water_depth_m,surface_height_m") {
                throw std::runtime_error("Unexpected FloodSim CSV header in " + input_path.string());
            }
            header_seen = true;
            frame.has_water_depth = true;
            continue;
        }

        const auto fields = split_csv_line(line);
        if (fields.size() != 5) {
            throw std::runtime_error("Malformed FloodSim CSV row in " + input_path.string());
        }
        frame.elevation.push_back(std::stod(fields[2]));
        frame.water_depth.push_back(std::stod(fields[3]));
        frame.surface_height.push_back(std::stod(fields[4]));
    }

    if (!header_seen) {
        throw std::runtime_error("Missing FloodSim CSV header in " + input_path.string());
    }
    if (frame.rows == 0 || frame.cols == 0) {
        throw std::runtime_error("FloodSim CSV is missing rows/cols metadata in " + input_path.string());
    }

    const std::size_t expected_cells = frame.rows * frame.cols;
    if (frame.elevation.size() != expected_cells ||
        frame.water_depth.size() != expected_cells ||
        frame.surface_height.size() != expected_cells) {
        throw std::runtime_error(
            "FloodSim CSV cell count does not match metadata in " + input_path.string());
    }

    frame.valid_cells = expected_cells;
    return frame;
}

RasterFrame load_terrain_frame(const std::filesystem::path& input_path) {
    const floodsim::LoadedTerrainRaster loaded =
        floodsim::load_terrain_raster_with_report(input_path.string());

    RasterFrame frame {
        .source_path = input_path,
        .source_format = "terrain_raster",
        .rows = loaded.terrain.rows,
        .cols = loaded.terrain.cols,
        .cell_size_m = loaded.terrain.cell_size_m,
        .valid_cells = loaded.terrain.valid_cell_count(),
        .has_water_depth = false,
        .origin_x_m = loaded.terrain.origin_x_m,
        .origin_y_m = loaded.terrain.origin_y_m,
        .crs_id = loaded.terrain.crs_id,
    };

    frame.elevation.reserve(frame.rows * frame.cols);
    for (std::size_t row = 0; row < frame.rows; ++row) {
        for (std::size_t col = 0; col < frame.cols; ++col) {
            const std::size_t index = row * frame.cols + col;
            if (loaded.terrain.valid_cell_mask.at(index) != 0) {
                frame.elevation.push_back(loaded.terrain.elevation_m.at(index));
            } else {
                frame.elevation.push_back(std::numeric_limits<double>::quiet_NaN());
            }
        }
    }
    return frame;
}

}  // namespace

RasterFrame load_raster_frame(const std::filesystem::path& input_path) {
    const std::string extension = input_path.extension().string();
    if (extension == ".csv") {
        return load_floodsim_csv_frame(input_path);
    }
    if (extension == ".tif" || extension == ".tiff" || extension == ".asc") {
        return load_terrain_frame(input_path);
    }

    throw std::runtime_error("Unsupported native viewer input format: " + input_path.string());
}

std::string format_raster_summary(const RasterFrame& summary) {
    std::ostringstream output;
    output << "source_path=\"" << summary.source_path.string() << "\"\n";
    output << "source_format=" << summary.source_format << '\n';
    output << "rows=" << summary.rows << '\n';
    output << "cols=" << summary.cols << '\n';
    output << "cell_size_m=" << summary.cell_size_m << '\n';
    output << "valid_cells=" << summary.valid_cells << '\n';
    output << "has_water_depth=" << (summary.has_water_depth ? "true" : "false") << '\n';
    if (summary.origin_x_m.has_value()) {
        output << "origin_x_m=" << *summary.origin_x_m << '\n';
    }
    if (summary.origin_y_m.has_value()) {
        output << "origin_y_m=" << *summary.origin_y_m << '\n';
    }
    if (summary.crs_id.has_value()) {
        output << "crs_id=" << *summary.crs_id << '\n';
    }
    if (summary.scenario_name.has_value()) {
        output << "scenario_name=" << *summary.scenario_name << '\n';
    }
    return output.str();
}

RasterLayer default_display_layer(const RasterFrame& frame) {
    return frame.has_water_depth ? RasterLayer::WaterDepth : RasterLayer::Elevation;
}

ColorImage make_color_image(const RasterFrame& frame, const RasterLayer layer) {
    const std::vector<double> dataset = dataset_for_layer(frame, layer);
    if (dataset.empty()) {
        throw std::runtime_error("Selected raster layer does not contain any data");
    }

    const auto [min_value, max_value] = finite_range(dataset);
    ColorImage image {
        .width = frame.cols,
        .height = frame.rows,
    };
    image.pixels_rgba8.reserve(dataset.size());
    for (const double value : dataset) {
        image.pixels_rgba8.push_back(
            color_for_layer_value(layer, value, min_value, max_value));
    }
    return image;
}

std::string raster_layer_name(const RasterLayer layer) {
    switch (layer) {
        case RasterLayer::Elevation:
            return "elevation";
        case RasterLayer::WaterDepth:
            return "water_depth";
        case RasterLayer::SurfaceHeight:
            return "surface_height";
    }

    throw std::runtime_error("Unhandled raster layer");
}

}  // namespace floodsim::native_viewer
