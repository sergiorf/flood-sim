#pragma once

#include <cstddef>
#include <cstdint>
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
    [[nodiscard]] double remaining_initial_loss(std::size_t row, std::size_t col) const;
    [[nodiscard]] bool is_cell_valid(std::size_t row, std::size_t col) const;
    [[nodiscard]] bool initial_loss_initialized() const noexcept;

    void set_elevation(std::size_t row, std::size_t col, double elevation_m);
    void set_water_depth(std::size_t row, std::size_t col, double water_depth_m);
    void add_water_depth(std::size_t row, std::size_t col, double delta_m);
    void set_initial_loss_remaining(std::size_t row, std::size_t col, double loss_m);
    void initialize_uniform_initial_loss(double loss_m);
    [[nodiscard]] double consume_initial_loss(std::size_t row, std::size_t col, double available_depth_m);
    void set_cell_valid(std::size_t row, std::size_t col, bool is_valid);

    [[nodiscard]] double total_water_depth() const noexcept;

private:
    [[nodiscard]] std::size_t index(std::size_t row, std::size_t col) const;

    std::size_t rows_;
    std::size_t cols_;
    double cell_size_m_;
    std::vector<double> elevation_m_;
    std::vector<double> water_depth_m_;
    std::vector<double> remaining_initial_loss_m_;
    std::vector<std::uint8_t> valid_cell_mask_;
    bool initial_loss_initialized_ {false};
};

}  // namespace floodsim
