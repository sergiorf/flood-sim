#include "floodsim/export.hpp"

#include <iomanip>
#include <ostream>

namespace floodsim {

void write_grid_csv(const Grid& grid, std::ostream& output) {
    output << "row,col,elevation_m,water_depth_m,surface_height_m\n";
    output << std::fixed << std::setprecision(6);

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            output << row << ','
                   << col << ','
                   << grid.elevation(row, col) << ','
                   << grid.water_depth(row, col) << ','
                   << grid.surface_height(row, col) << '\n';
        }
    }
}

}  // namespace floodsim
