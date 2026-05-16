#include "graphics_backend.hpp"
#include "raster_frame.hpp"

#include <iostream>
#include <stdexcept>

namespace {

std::string usage_message() {
    return
        "Usage: floodsim_native_viewer <input>\n"
        "       floodsim_native_viewer --inspect <input>\n"
        "       floodsim_native_viewer --show <input>\n"
        "\n"
        "Current scope:\n"
        "- inspect terrain rasters such as .tif, .tiff, and .asc\n"
        "- inspect FloodSim CSV result rasters\n"
        "- display one raster layer through the current graphics backend\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 2 && argc != 3) {
            throw std::runtime_error(usage_message());
        }

        bool inspect_only = false;
        std::string input_path;
        if (argc == 2) {
            input_path = argv[1];
        } else {
            const std::string option = argv[1];
            if (option == "--inspect") {
                inspect_only = true;
            } else if (option != "--show") {
                throw std::runtime_error("Unknown option: " + option + "\n" + usage_message());
            }
            input_path = argv[2];
        }

        const auto frame = floodsim::native_viewer::load_raster_frame(input_path);
        if (inspect_only) {
            std::cout << "floodsim_native_viewer_scaffold=true\n";
            std::cout << floodsim::native_viewer::format_raster_summary(frame);
            std::cout << "default_layer="
                      << floodsim::native_viewer::raster_layer_name(
                             floodsim::native_viewer::default_display_layer(frame))
                      << '\n';
            std::cout << "next_step=add_image_based_native_rendering\n";
            return 0;
        }

        const auto layer = floodsim::native_viewer::default_display_layer(frame);
        const auto image = floodsim::native_viewer::make_color_image(frame, layer);
        auto backend = floodsim::native_viewer::make_default_graphics_backend();
        return backend->show_image(
            floodsim::native_viewer::GraphicsWindowConfig {
                .title = "FloodSim Native Viewer - " +
                    floodsim::native_viewer::raster_layer_name(layer),
                .window_width = 1280,
                .window_height = 900,
            },
            image);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
