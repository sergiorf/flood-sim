#include "floodsim/export.hpp"

#include <iomanip>
#include <ostream>

namespace floodsim {

void write_grid_csv(const Grid& grid, std::ostream& output, const GridCsvMetadata& metadata) {
    output << std::fixed << std::setprecision(6);
    output << "# floodsim_csv_version,1\n";
    output << "# rows," << grid.rows() << '\n';
    output << "# cols," << grid.cols() << '\n';
    output << "# cell_size_m," << grid.cell_size_m() << '\n';
    if (metadata.origin_x_m.has_value() && metadata.origin_y_m.has_value()) {
        output << "# origin_x_m," << *metadata.origin_x_m << '\n';
        output << "# origin_y_m," << *metadata.origin_y_m << '\n';
    }
    if (metadata.crs_id.has_value()) {
        output << "# crs_id," << *metadata.crs_id << '\n';
    }
    output << "row,col,elevation_m,water_depth_m,surface_height_m\n";

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
