#include "graphics_backend.hpp"

#include <SDL.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace floodsim::native_viewer {

namespace {

class SdlGraphicsBackend final : public GraphicsBackend {
public:
    int show_image(
        const GraphicsWindowConfig& config,
        const ColorImage& image) override {
        if (image.width == 0 || image.height == 0 || image.pixels_rgba8.empty()) {
            throw std::runtime_error("Cannot display an empty image");
        }

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }

        SDL_Window* window = SDL_CreateWindow(
            config.title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.window_width,
            config.window_height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (window == nullptr) {
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC,
            static_cast<int>(image.width),
            static_cast<int>(image.height));
        if (texture == nullptr) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
        }

        if (SDL_UpdateTexture(
                texture,
                nullptr,
                image.pixels_rgba8.data(),
                static_cast<int>(image.width * sizeof(std::uint32_t))) != 0) {
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("SDL_UpdateTexture failed: " + std::string(SDL_GetError()));
        }

        int exit_code = 0;
        bool running = true;
        while (running) {
            SDL_Event event {};
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }

            int window_width = 0;
            int window_height = 0;
            SDL_GetWindowSize(window, &window_width, &window_height);

            const float scale_x =
                static_cast<float>(window_width) / static_cast<float>(image.width);
            const float scale_y =
                static_cast<float>(window_height) / static_cast<float>(image.height);
            const float scale = scale_x < scale_y ? scale_x : scale_y;
            const int draw_width = static_cast<int>(static_cast<float>(image.width) * scale);
            const int draw_height = static_cast<int>(static_cast<float>(image.height) * scale);
            SDL_Rect destination {
                .x = (window_width - draw_width) / 2,
                .y = (window_height - draw_height) / 2,
                .w = draw_width,
                .h = draw_height,
            };

            SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, &destination);
            SDL_RenderPresent(renderer);
        }

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return exit_code;
    }
};

}  // namespace

std::unique_ptr<GraphicsBackend> make_default_graphics_backend() {
    return std::make_unique<SdlGraphicsBackend>();
}

}  // namespace floodsim::native_viewer
