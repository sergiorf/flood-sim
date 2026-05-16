#pragma once

#include <filesystem>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace floodsim::native_viewer {

enum class RasterLayer {
    Elevation,
    WaterDepth,
    SurfaceHeight,
};

struct RasterFrame {
    std::filesystem::path source_path;
    std::string source_format;
    std::size_t rows {0};
    std::size_t cols {0};
    double cell_size_m {0.0};
    std::size_t valid_cells {0};
    bool has_water_depth {false};
    std::optional<double> origin_x_m;
    std::optional<double> origin_y_m;
    std::optional<std::string> crs_id;
    std::optional<std::string> scenario_name;
    std::vector<double> elevation;
    std::vector<double> water_depth;
    std::vector<double> surface_height;
};

struct ColorImage {
    std::size_t width {0};
    std::size_t height {0};
    std::vector<std::uint32_t> pixels_rgba8;
};

[[nodiscard]] RasterFrame load_raster_frame(const std::filesystem::path& input_path);
[[nodiscard]] std::string format_raster_summary(const RasterFrame& summary);
[[nodiscard]] RasterLayer default_display_layer(const RasterFrame& frame);
[[nodiscard]] ColorImage make_color_image(const RasterFrame& frame, RasterLayer layer);
[[nodiscard]] std::string raster_layer_name(RasterLayer layer);

}  // namespace floodsim::native_viewer
