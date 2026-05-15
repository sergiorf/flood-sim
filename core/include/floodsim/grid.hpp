#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace floodsim {

// Grid is the in-memory simulation state for one raster domain.
//
// It stores static terrain elevation plus dynamic per-cell state such as
// ponded water depth and remaining event-start loss. The grid owns the valid
// cell mask so imported DEM nodata can stay in raster coordinates without
// participating in rainfall or routing.
class Grid {
public:
    Grid(std::size_t rows, std::size_t cols, double cell_size_m = 1.0);

    // Raster shape and nominal square cell size in meters.
    [[nodiscard]] std::size_t rows() const noexcept;
    [[nodiscard]] std::size_t cols() const noexcept;
    [[nodiscard]] double cell_size_m() const noexcept;

    // Cell state accessors. surface_height() is elevation + water depth.
    [[nodiscard]] double elevation(std::size_t row, std::size_t col) const;
    [[nodiscard]] double water_depth(std::size_t row, std::size_t col) const;
    [[nodiscard]] double surface_height(std::size_t row, std::size_t col) const;

    // Remaining per-cell event-start loss store. This is consumed before new
    // rainfall becomes surface water when initial loss is enabled.
    [[nodiscard]] double remaining_initial_loss(std::size_t row, std::size_t col) const;
    [[nodiscard]] bool is_cell_valid(std::size_t row, std::size_t col) const;
    [[nodiscard]] bool initial_loss_initialized() const noexcept;

    // Direct state mutation helpers used by setup, tests, and export adapters.
    void set_elevation(std::size_t row, std::size_t col, double elevation_m);
    void set_water_depth(std::size_t row, std::size_t col, double water_depth_m);
    void add_water_depth(std::size_t row, std::size_t col, double delta_m);
    void set_initial_loss_remaining(std::size_t row, std::size_t col, double loss_m);

    // Initialize every valid cell with the same event-start loss depth.
    void initialize_uniform_initial_loss(double loss_m);

    // Consume available rainfall depth against the cell's remaining initial
    // loss store and return the residual depth that should become runoff.
    [[nodiscard]] double consume_initial_loss(std::size_t row, std::size_t col, double available_depth_m);
    void set_cell_valid(std::size_t row, std::size_t col, bool is_valid);

    // Sum of ponded surface water depth over every valid cell.
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
