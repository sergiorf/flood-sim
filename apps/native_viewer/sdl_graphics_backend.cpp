#include "graphics_backend.hpp"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace floodsim::native_viewer {

namespace {

struct ViewerState {
    std::size_t frame_index {0};
    RasterLayer layer {RasterLayer::Elevation};
    double zoom {1.0};
    double pan_x {0.0};
    double pan_y {0.0};
    bool dragging {false};
    int last_drag_x {0};
    int last_drag_y {0};
    std::optional<std::size_t> selected_row;
    std::optional<std::size_t> selected_col;
};

class SdlSession final {
public:
    SdlSession() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }
    }

    ~SdlSession() {
        SDL_Quit();
    }
};

class SdlWindow final {
public:
    SdlWindow(const GraphicsWindowConfig& config, const ColorImage& image) {
        window_ = SDL_CreateWindow(
            config.title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.window_width,
            config.window_height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (window_ == nullptr) {
            throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }

        renderer_ = SDL_CreateRenderer(
            window_,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer_ == nullptr) {
            throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
        }

        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC,
            static_cast<int>(image.width),
            static_cast<int>(image.height));
        if (texture_ == nullptr) {
            throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
        }
    }

    ~SdlWindow() {
        if (texture_ != nullptr) {
            SDL_DestroyTexture(texture_);
        }
        if (renderer_ != nullptr) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_ != nullptr) {
            SDL_DestroyWindow(window_);
        }
    }

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;

    SDL_Window* window() const noexcept { return window_; }
    SDL_Renderer* renderer() const noexcept { return renderer_; }
    SDL_Texture* texture() const noexcept { return texture_; }

private:
    SDL_Window* window_ {nullptr};
    SDL_Renderer* renderer_ {nullptr};
    SDL_Texture* texture_ {nullptr};
};

std::string format_float(const double value, const int precision = 3) {
    std::ostringstream output;
    output.setf(std::ios::fixed);
    output.precision(precision);
    output << value;
    return output.str();
}

std::string format_value(const double value) {
    if (std::isnan(value)) {
        return "nodata";
    }
    return format_float(value);
}

std::size_t next_frame_index(const RasterDocument& document, const std::size_t current, const int delta) {
    if (document.frames.size() <= 1) {
        return current;
    }
    const int count = static_cast<int>(document.frames.size());
    int next = static_cast<int>(current) + delta;
    if (next < 0) {
        next = 0;
    }
    if (next >= count) {
        next = count - 1;
    }
    return static_cast<std::size_t>(next);
}

RasterLayer next_available_layer(const RasterFrame& frame, const RasterLayer current) {
    const std::vector<RasterLayer> layers = available_layers(frame);
    const auto found = std::find(layers.begin(), layers.end(), current);
    if (found == layers.end()) {
        return layers.front();
    }
    const std::size_t index = static_cast<std::size_t>(std::distance(layers.begin(), found));
    return layers[(index + 1) % layers.size()];
}

void update_texture(SDL_Texture* texture, const ColorImage& image) {
    if (SDL_UpdateTexture(
            texture,
            nullptr,
            image.pixels_rgba8.data(),
            static_cast<int>(image.width * sizeof(std::uint32_t))) != 0) {
        throw std::runtime_error("SDL_UpdateTexture failed: " + std::string(SDL_GetError()));
    }
}

SDL_FRect compute_destination_rect(
    const RasterFrame& frame,
    const ViewerState& state,
    const int window_width,
    const int window_height) {
    const double fit_scale_x = static_cast<double>(window_width) / static_cast<double>(frame.cols);
    const double fit_scale_y = static_cast<double>(window_height) / static_cast<double>(frame.rows);
    const double fit_scale = std::min(fit_scale_x, fit_scale_y);
    const double scale = fit_scale * state.zoom;
    const double draw_width = static_cast<double>(frame.cols) * scale;
    const double draw_height = static_cast<double>(frame.rows) * scale;
    return SDL_FRect {
        static_cast<float>(((window_width - draw_width) * 0.5) + state.pan_x),
        static_cast<float>(((window_height - draw_height) * 0.5) + state.pan_y),
        static_cast<float>(draw_width),
        static_cast<float>(draw_height),
    };
}

std::optional<std::pair<std::size_t, std::size_t>> pick_cell(
    const RasterFrame& frame,
    const SDL_FRect& rect,
    const int mouse_x,
    const int mouse_y) {
    if (rect.w <= 0.0f || rect.h <= 0.0f) {
        return std::nullopt;
    }
    if (mouse_x < rect.x || mouse_y < rect.y ||
        mouse_x >= rect.x + rect.w || mouse_y >= rect.y + rect.h) {
        return std::nullopt;
    }

    const double normalized_x = (static_cast<double>(mouse_x) - rect.x) / rect.w;
    const double normalized_y = (static_cast<double>(mouse_y) - rect.y) / rect.h;
    const std::size_t col = std::min(
        frame.cols - 1,
        static_cast<std::size_t>(normalized_x * static_cast<double>(frame.cols)));
    const std::size_t row = std::min(
        frame.rows - 1,
        static_cast<std::size_t>(normalized_y * static_cast<double>(frame.rows)));
    return std::pair<std::size_t, std::size_t> {row, col};
}

void draw_selection_overlay(
    SDL_Renderer* renderer,
    const RasterFrame& frame,
    const SDL_FRect& rect,
    const ViewerState& state) {
    if (!state.selected_row.has_value() || !state.selected_col.has_value()) {
        return;
    }

    const float cell_width = rect.w / static_cast<float>(frame.cols);
    const float cell_height = rect.h / static_cast<float>(frame.rows);
    SDL_FRect highlight {
        rect.x + (static_cast<float>(*state.selected_col) * cell_width),
        rect.y + (static_cast<float>(*state.selected_row) * cell_height),
        std::max(1.0f, cell_width),
        std::max(1.0f, cell_height),
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 64);
    SDL_RenderFillRectF(renderer, &highlight);
    SDL_SetRenderDrawColor(renderer, 255, 80, 20, 255);
    SDL_RenderDrawRectF(renderer, &highlight);
}

std::string build_window_title(
    const GraphicsWindowConfig& config,
    const RasterDocument& document,
    const ViewerState& state) {
    const RasterFrame& frame = document.frames.at(state.frame_index);
    std::ostringstream title;
    title << config.title
          << " | frame " << (state.frame_index + 1) << '/' << document.frames.size()
          << " | layer " << raster_layer_name(state.layer)
          << " | zoom " << format_float(state.zoom, 2)
          << " | " << frame.cols << "x" << frame.rows;

    if (frame.scenario_name.has_value()) {
        title << " | scenario " << *frame.scenario_name;
    }
    if (state.selected_row.has_value() && state.selected_col.has_value()) {
        title << " | "
              << format_cell_details(frame, state.layer, *state.selected_row, *state.selected_col);
    }
    return title.str();
}

void print_controls_once() {
    std::cout
        << "native_viewer_controls: "
        << "mousewheel=zoom "
        << "drag=pan "
        << "left_click=inspect_cell "
        << "left_right=step_frames "
        << "tab=cycle_layers "
        << "1/2/3=select_layer "
        << "0=reset_view "
        << "esc=quit\n";
}

class SdlGraphicsBackend final : public GraphicsBackend {
public:
    int show_document(
        const GraphicsWindowConfig& config,
        const RasterDocument& document) override {
        if (document.frames.empty()) {
            throw std::runtime_error("Cannot display an empty raster document");
        }

        print_controls_once();
        SdlSession session;
        ViewerState state {
            .frame_index = 0,
            .layer = default_display_layer(document.frames.front()),
        };

        ColorImage current_image = make_color_image(document.frames.front(), state.layer);
        SdlWindow window(config, current_image);
        update_texture(window.texture(), current_image);
        SDL_SetWindowTitle(window.window(), build_window_title(config, document, state).c_str());

        bool running = true;
        while (running) {
            SDL_Event event {};
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    running = false;
                    continue;
                }

                const RasterFrame& frame = document.frames.at(state.frame_index);
                bool refresh_texture = false;
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_LEFT:
                            state.frame_index = next_frame_index(document, state.frame_index, -1);
                            state.layer = default_display_layer(document.frames.at(state.frame_index));
                            state.selected_row.reset();
                            state.selected_col.reset();
                            refresh_texture = true;
                            break;
                        case SDLK_RIGHT:
                            state.frame_index = next_frame_index(document, state.frame_index, 1);
                            state.layer = default_display_layer(document.frames.at(state.frame_index));
                            state.selected_row.reset();
                            state.selected_col.reset();
                            refresh_texture = true;
                            break;
                        case SDLK_TAB:
                            state.layer = next_available_layer(frame, state.layer);
                            refresh_texture = true;
                            break;
                        case SDLK_1:
                            state.layer = RasterLayer::Elevation;
                            refresh_texture = true;
                            break;
                        case SDLK_2:
                            if (frame_has_layer(frame, RasterLayer::WaterDepth)) {
                                state.layer = RasterLayer::WaterDepth;
                                refresh_texture = true;
                            }
                            break;
                        case SDLK_3:
                            if (frame_has_layer(frame, RasterLayer::SurfaceHeight)) {
                                state.layer = RasterLayer::SurfaceHeight;
                                refresh_texture = true;
                            }
                            break;
                        case SDLK_0:
                            state.zoom = 1.0;
                            state.pan_x = 0.0;
                            state.pan_y = 0.0;
                            break;
                        default:
                            break;
                    }
                } else if (event.type == SDL_MOUSEWHEEL) {
                    const double factor = event.wheel.y > 0 ? 1.2 : 1.0 / 1.2;
                    state.zoom = std::clamp(state.zoom * factor, 0.25, 40.0);
                } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    state.dragging = true;
                    state.last_drag_x = event.button.x;
                    state.last_drag_y = event.button.y;

                    int window_width = 0;
                    int window_height = 0;
                    SDL_GetWindowSize(window.window(), &window_width, &window_height);
                    const SDL_FRect rect = compute_destination_rect(frame, state, window_width, window_height);
                    if (const auto picked = pick_cell(frame, rect, event.button.x, event.button.y); picked.has_value()) {
                        state.selected_row = picked->first;
                        state.selected_col = picked->second;
                        std::cout << format_cell_details(frame, state.layer, *state.selected_row, *state.selected_col) << '\n';
                    }
                } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                    state.dragging = false;
                } else if (event.type == SDL_MOUSEMOTION && state.dragging) {
                    state.pan_x += static_cast<double>(event.motion.x - state.last_drag_x);
                    state.pan_y += static_cast<double>(event.motion.y - state.last_drag_y);
                    state.last_drag_x = event.motion.x;
                    state.last_drag_y = event.motion.y;
                }

                if (refresh_texture) {
                    const RasterFrame& refreshed_frame = document.frames.at(state.frame_index);
                    current_image = make_color_image(refreshed_frame, state.layer);
                    update_texture(window.texture(), current_image);
                }

                SDL_SetWindowTitle(window.window(), build_window_title(config, document, state).c_str());
            }

            int window_width = 0;
            int window_height = 0;
            SDL_GetWindowSize(window.window(), &window_width, &window_height);
            const RasterFrame& frame = document.frames.at(state.frame_index);
            const SDL_FRect destination = compute_destination_rect(frame, state, window_width, window_height);

            SDL_SetRenderDrawColor(window.renderer(), 18, 18, 18, 255);
            SDL_RenderClear(window.renderer());
            SDL_RenderCopyF(window.renderer(), window.texture(), nullptr, &destination);
            draw_selection_overlay(window.renderer(), frame, destination, state);
            SDL_RenderPresent(window.renderer());
        }

        return 0;
    }
};

}  // namespace

std::unique_ptr<GraphicsBackend> make_default_graphics_backend() {
    return std::make_unique<SdlGraphicsBackend>();
}

}  // namespace floodsim::native_viewer
