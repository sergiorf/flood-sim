#include "floodsim/grid.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdexcept>

namespace floodsim {

Grid::Grid(std::size_t rows, std::size_t cols, double cell_size_m)
    : rows_(rows),
      cols_(cols),
      cell_size_m_(cell_size_m),
      elevation_m_(rows * cols, 0.0),
      water_depth_m_(rows * cols, 0.0),
      valid_cell_mask_(rows * cols, 1) {
    if (rows_ == 0 || cols_ == 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    if (cell_size_m_ <= 0.0) {
        throw std::invalid_argument("Cell size must be positive");
    }
}

std::size_t Grid::rows() const noexcept {
    return rows_;
}

std::size_t Grid::cols() const noexcept {
    return cols_;
}

double Grid::cell_size_m() const noexcept {
    return cell_size_m_;
}

double Grid::elevation(std::size_t row, std::size_t col) const {
    return elevation_m_.at(index(row, col));
}

double Grid::water_depth(std::size_t row, std::size_t col) const {
    return water_depth_m_.at(index(row, col));
}

double Grid::surface_height(std::size_t row, std::size_t col) const {
    return elevation(row, col) + water_depth(row, col);
}

bool Grid::is_cell_valid(std::size_t row, std::size_t col) const {
    return valid_cell_mask_.at(index(row, col)) != 0;
}

void Grid::set_elevation(std::size_t row, std::size_t col, double elevation_m) {
    elevation_m_.at(index(row, col)) = elevation_m;
}

void Grid::set_water_depth(std::size_t row, std::size_t col, double water_depth_m) {
    const auto idx = index(row, col);
    water_depth_m_.at(idx) = valid_cell_mask_.at(idx) != 0 ? std::max(0.0, water_depth_m) : 0.0;
}

void Grid::add_water_depth(std::size_t row, std::size_t col, double delta_m) {
    const auto idx = index(row, col);
    if (valid_cell_mask_.at(idx) == 0) {
        water_depth_m_.at(idx) = 0.0;
        return;
    }

    water_depth_m_.at(idx) = std::max(0.0, water_depth_m_.at(idx) + delta_m);
}

void Grid::set_cell_valid(std::size_t row, std::size_t col, bool is_valid) {
    const auto idx = index(row, col);
    valid_cell_mask_.at(idx) = is_valid ? 1 : 0;
    if (!is_valid) {
        water_depth_m_.at(idx) = 0.0;
    }
}

double Grid::total_water_depth() const noexcept {
    return std::accumulate(water_depth_m_.begin(), water_depth_m_.end(), 0.0);
}

std::size_t Grid::index(std::size_t row, std::size_t col) const {
    assert(row < rows_);
    assert(col < cols_);
    return row * cols_ + col;
}

}  // namespace floodsim
