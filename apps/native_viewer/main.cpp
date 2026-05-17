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
        "- pan and zoom\n"
        "- click for cell inspection\n"
        "- step snapshot series from FloodSim CSV outputs\n";
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

        const auto document = floodsim::native_viewer::load_raster_document(input_path);
        const auto& frame = document.frames.front();
        if (inspect_only) {
            std::cout << "floodsim_native_viewer_scaffold=false\n";
            std::cout << "frame_count=" << document.frames.size() << '\n';
            std::cout << floodsim::native_viewer::format_raster_summary(frame);
            std::cout << "default_layer="
                      << floodsim::native_viewer::raster_layer_name(
                             floodsim::native_viewer::default_display_layer(frame))
                      << '\n';
            return 0;
        }

        auto backend = floodsim::native_viewer::make_default_graphics_backend();
        return backend->show_document(
            floodsim::native_viewer::GraphicsWindowConfig {
                .title = "FloodSim Native Viewer",
                .window_width = 1280,
                .window_height = 900,
            },
            document);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
