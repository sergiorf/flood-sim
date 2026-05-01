#pragma once

#include <cstddef>
#include <vector>

namespace floodsim {

class Grid {
public:
    Grid(std::size_t rows, std::size_t cols, double cell_size_m = 1.0);

    [[nodiscard]] std::size_t rows() const noexcept;
    [[nodiscard]] std::size_t cols() const noexcept;
    [[nodiscard]] double cell_size_m() const noexcept;

    [[nodiscard]] double elevation(std::size_t row, std::size_t col) const;
    [[nodiscard]] double water_depth(std::size_t row, std::size_t col) const;
    [[nodiscard]] double surface_height(std::size_t row, std::size_t col) const;

    void set_elevation(std::size_t row, std::size_t col, double elevation_m);
    void set_water_depth(std::size_t row, std::size_t col, double water_depth_m);
    void add_water_depth(std::size_t row, std::size_t col, double delta_m);

    [[nodiscard]] double total_water_depth() const noexcept;

private:
    [[nodiscard]] std::size_t index(std::size_t row, std::size_t col) const;

    std::size_t rows_;
    std::size_t cols_;
    double cell_size_m_;
    std::vector<double> elevation_m_;
    std::vector<double> water_depth_m_;
};

}  // namespace floodsim

