#pragma once

#include "raster_frame.hpp"

#include <memory>
#include <string>

namespace floodsim::native_viewer {

struct GraphicsWindowConfig {
    std::string title;
    int window_width {1280};
    int window_height {900};
};

class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;

    virtual int show_document(
        const GraphicsWindowConfig& config,
        const RasterDocument& document) = 0;
};

[[nodiscard]] std::unique_ptr<GraphicsBackend> make_default_graphics_backend();

}  // namespace floodsim::native_viewer
