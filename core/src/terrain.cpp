#include "floodsim/terrain.hpp"

#include "floodsim/grid.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

#if FLOODSIM_HAS_GDAL
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#endif

namespace floodsim {

namespace {

LoadedTerrainRaster load_terrain_raster_impl(const std::string& path, const TerrainWindow& window);

}  // namespace

std::size_t TerrainRaster::cell_count() const noexcept {
    return rows * cols;
}

std::size_t TerrainRaster::valid_cell_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        valid_cell_mask.begin(),
        valid_cell_mask.end(),
        [](std::uint8_t value) { return value != 0; }));
}

void validate_terrain_raster(const TerrainRaster& terrain) {
    if (terrain.rows == 0 || terrain.cols == 0) {
        throw std::invalid_argument("Terrain raster dimensions must be positive");
    }
    if (terrain.cell_size_m <= 0.0) {
        throw std::invalid_argument("Terrain raster cell size must be positive");
    }

    const std::size_t expected_cells = terrain.cell_count();
    if (terrain.elevation_m.size() != expected_cells) {
        throw std::invalid_argument("Terrain raster elevation array size does not match rows * cols");
    }
    if (terrain.valid_cell_mask.size() != expected_cells) {
        throw std::invalid_argument("Terrain raster valid-cell mask size does not match rows * cols");
    }

    const bool has_origin_x = terrain.origin_x_m.has_value();
    const bool has_origin_y = terrain.origin_y_m.has_value();
    if (has_origin_x != has_origin_y) {
        throw std::invalid_argument("Terrain raster origin metadata must provide both x and y or neither");
    }

    const bool has_any_valid_cell = std::any_of(
        terrain.valid_cell_mask.begin(),
        terrain.valid_cell_mask.end(),
        [](std::uint8_t value) { return value != 0; });
    if (!has_any_valid_cell) {
        throw std::invalid_argument("Terrain raster must contain at least one valid simulation cell");
    }
}

void validate_terrain_window(const TerrainWindow& window) {
    if (window.rows == 0 || window.cols == 0) {
        throw std::invalid_argument("Terrain window dimensions must be positive");
    }
}

Grid make_grid_from_terrain(const TerrainRaster& terrain) {
    validate_terrain_raster(terrain);

    Grid grid(terrain.rows, terrain.cols, terrain.cell_size_m);
    for (std::size_t row = 0; row < terrain.rows; ++row) {
        for (std::size_t col = 0; col < terrain.cols; ++col) {
            const std::size_t index = (row * terrain.cols) + col;
            grid.set_elevation(row, col, terrain.elevation_m.at(index));
            grid.set_cell_valid(row, col, terrain.valid_cell_mask.at(index) != 0);
        }
    }

    return grid;
}

TerrainRaster load_terrain_raster_from_file(const std::string& path) {
    return load_terrain_raster_with_report(path).terrain;
}

TerrainRaster load_terrain_raster_from_file(const std::string& path, const TerrainWindow& window) {
    return load_terrain_raster_with_report(path, window).terrain;
}

LoadedTerrainRaster load_terrain_raster_with_report(const std::string& path) {
    return load_terrain_raster_with_report(path, TerrainWindow {});
}

LoadedTerrainRaster load_terrain_raster_with_report(const std::string& path, const TerrainWindow& window) {
    return load_terrain_raster_impl(path, window);
}

namespace {

LoadedTerrainRaster load_terrain_raster_impl(const std::string& path, const TerrainWindow& window) {
#if !FLOODSIM_HAS_GDAL
    (void)path;
    (void)window;
    throw std::runtime_error("GDAL support is disabled for this build");
#else
    GDALAllRegister();

    std::unique_ptr<GDALDataset, decltype(&GDALClose)> dataset(
        static_cast<GDALDataset*>(GDALOpen(path.c_str(), GA_ReadOnly)),
        GDALClose);
    if (!dataset) {
        throw std::runtime_error("Failed to open terrain raster with GDAL");
    }

    if (dataset->GetRasterCount() != 1) {
        throw std::invalid_argument("The Phase 2 GDAL terrain loader supports single-band rasters only");
    }

    double geotransform[6] {};
    if (dataset->GetGeoTransform(geotransform) != CE_None) {
        throw std::invalid_argument("Terrain raster must provide a valid geotransform");
    }

    const double pixel_width_m = geotransform[1];
    const double pixel_height_m = std::fabs(geotransform[5]);
    if (pixel_width_m <= 0.0 || pixel_height_m <= 0.0) {
        throw std::invalid_argument("Terrain raster must have positive pixel spacing");
    }
    if (std::fabs(geotransform[2]) > 1e-12 || std::fabs(geotransform[4]) > 1e-12) {
        throw std::invalid_argument("The Phase 2 GDAL terrain loader does not support rotated or sheared rasters");
    }
    if (std::fabs(pixel_width_m - pixel_height_m) > 1e-9) {
        throw std::invalid_argument("The Phase 2 GDAL terrain loader supports square pixels only");
    }

    GDALRasterBand* band = dataset->GetRasterBand(1);
    if (!band) {
        throw std::runtime_error("Failed to access the first raster band");
    }

    const std::size_t dataset_rows = static_cast<std::size_t>(dataset->GetRasterYSize());
    const std::size_t dataset_cols = static_cast<std::size_t>(dataset->GetRasterXSize());

    TerrainWindow effective_window {
        .row_offset = 0,
        .col_offset = 0,
        .rows = dataset_rows,
        .cols = dataset_cols,
    };
    if (window.rows != 0 || window.cols != 0) {
        validate_terrain_window(window);
        if (window.row_offset >= dataset_rows || window.col_offset >= dataset_cols) {
            throw std::invalid_argument("Terrain window origin must lie within the source raster");
        }
        if (window.rows > (dataset_rows - window.row_offset) ||
            window.cols > (dataset_cols - window.col_offset)) {
            throw std::invalid_argument("Terrain window extends beyond the source raster bounds");
        }
        effective_window = window;
    }

    LoadedTerrainRaster loaded;
    TerrainRaster& terrain = loaded.terrain;
    TerrainIngestionReport& report = loaded.report;

    report.source_rows = dataset_rows;
    report.source_cols = dataset_cols;
    report.loaded_rows = effective_window.rows;
    report.loaded_cols = effective_window.cols;
    report.window_applied =
        effective_window.row_offset != 0 ||
        effective_window.col_offset != 0 ||
        effective_window.rows != dataset_rows ||
        effective_window.cols != dataset_cols;
    report.clipped_cell_count = (dataset_rows * dataset_cols) - (effective_window.rows * effective_window.cols);

    terrain.rows = effective_window.rows;
    terrain.cols = effective_window.cols;
    terrain.cell_size_m = pixel_width_m;
    terrain.origin_x_m = geotransform[0] + (static_cast<double>(effective_window.col_offset) * geotransform[1]);
    terrain.origin_y_m = geotransform[3] + (static_cast<double>(effective_window.row_offset) * geotransform[5]);
    terrain.elevation_m.resize(terrain.cell_count());
    terrain.valid_cell_mask.assign(terrain.cell_count(), 1);

    const CPLErr read_error = band->RasterIO(
        GF_Read,
        static_cast<int>(effective_window.col_offset),
        static_cast<int>(effective_window.row_offset),
        static_cast<int>(effective_window.cols),
        static_cast<int>(effective_window.rows),
        terrain.elevation_m.data(),
        static_cast<int>(terrain.cols),
        static_cast<int>(terrain.rows),
        GDT_Float64,
        0,
        0);
    if (read_error != CE_None) {
        throw std::runtime_error("Failed to read terrain raster values");
    }

    int nodata_is_set = 0;
    const double nodata_value = band->GetNoDataValue(&nodata_is_set);
    report.nodata_metadata_present = nodata_is_set != 0;
    if (nodata_is_set != 0) {
        report.nodata_status = TerrainNodataStatus::BandMetadataApplied;
        for (std::size_t idx = 0; idx < terrain.elevation_m.size(); ++idx) {
            const double elevation = terrain.elevation_m[idx];
            if (std::isnan(elevation)) {
                ++report.nan_cell_count;
            }
            const bool is_nodata =
                (std::isnan(nodata_value) && std::isnan(elevation)) ||
                (!std::isnan(nodata_value) && elevation == nodata_value);
            terrain.valid_cell_mask[idx] = is_nodata ? 0 : 1;
        }
    } else {
        report.nan_cell_count = static_cast<std::size_t>(std::count_if(
            terrain.elevation_m.begin(),
            terrain.elevation_m.end(),
            [](double elevation) { return std::isnan(elevation); }));
        report.nodata_status =
            report.nan_cell_count == 0
            ? TerrainNodataStatus::BandMetadataMissingAllCellsValid
            : TerrainNodataStatus::BandMetadataMissingNaNCellsPresent;
    }

    const char* projection_ref = dataset->GetProjectionRef();
    if (projection_ref && projection_ref[0] != '\0') {
        OGRSpatialReference spatial_ref(projection_ref);
        const char* authority_name = spatial_ref.GetAuthorityName(nullptr);
        const char* authority_code = spatial_ref.GetAuthorityCode(nullptr);

        if (authority_name && authority_code) {
            terrain.crs_id = std::string(authority_name) + ":" + authority_code;
        } else {
            terrain.crs_id = std::string(projection_ref);
        }
    }

    validate_terrain_raster(terrain);
    report.valid_cell_count = terrain.valid_cell_count();
    report.invalid_cell_count = terrain.cell_count() - report.valid_cell_count;
    return loaded;
#endif
}

}  // namespace

}  // namespace floodsim
