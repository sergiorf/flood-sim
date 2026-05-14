#include "floodsim/export.hpp"
#include "floodsim/terrain.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc != 3) {
            throw std::runtime_error(
                "Usage: floodsim_terrain_debug_export <input_raster> <output.csv>");
        }

        const std::string input_path = argv[1];
        const std::string output_path = argv[2];
        const floodsim::LoadedTerrainRaster loaded =
            floodsim::load_terrain_raster_with_report(input_path);
        const floodsim::Grid grid = floodsim::make_grid_from_terrain(loaded.terrain);

        std::ofstream output(output_path);
        if (!output) {
            throw std::runtime_error("Failed to open terrain debug CSV output path");
        }

        floodsim::write_grid_csv(
            grid,
            output,
            floodsim::GridCsvMetadata {
                .origin_x_m = loaded.terrain.origin_x_m,
                .origin_y_m = loaded.terrain.origin_y_m,
                .crs_id = loaded.terrain.crs_id,
            });
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
