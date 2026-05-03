#include "floodsim/terrain.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

#if FLOODSIM_HAS_GDAL
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#endif

namespace floodsim {

std::size_t TerrainRaster::cell_count() const noexcept {
    return rows * cols;
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

TerrainRaster load_terrain_raster_from_file(const std::string& path) {
#if !FLOODSIM_HAS_GDAL
    (void)path;
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

    TerrainRaster terrain;
    terrain.rows = static_cast<std::size_t>(dataset->GetRasterYSize());
    terrain.cols = static_cast<std::size_t>(dataset->GetRasterXSize());
    terrain.cell_size_m = pixel_width_m;
    terrain.origin_x_m = geotransform[0];
    terrain.origin_y_m = geotransform[3];
    terrain.elevation_m.resize(terrain.cell_count());
    terrain.valid_cell_mask.assign(terrain.cell_count(), 1);

    const CPLErr read_error = band->RasterIO(
        GF_Read,
        0,
        0,
        static_cast<int>(terrain.cols),
        static_cast<int>(terrain.rows),
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
    if (nodata_is_set != 0) {
        for (std::size_t idx = 0; idx < terrain.elevation_m.size(); ++idx) {
            const double elevation = terrain.elevation_m[idx];
            const bool is_nodata =
                (std::isnan(nodata_value) && std::isnan(elevation)) ||
                (!std::isnan(nodata_value) && elevation == nodata_value);
            terrain.valid_cell_mask[idx] = is_nodata ? 0 : 1;
        }
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
    return terrain;
#endif
}

}  // namespace floodsim
