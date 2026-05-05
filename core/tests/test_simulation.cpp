#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "floodsim/export.hpp"
#include "floodsim/grid.hpp"
#include "floodsim/simulation.hpp"
#include "floodsim/terrain.hpp"

#include <filesystem>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#if FLOODSIM_HAS_GDAL
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#endif

namespace {

using floodsim::BoundaryMode;
using floodsim::Grid;
using floodsim::RainfallScenario;
using floodsim::SimulationConfig;
using floodsim::TerrainRaster;

bool nearly_equal(double lhs, double rhs, double epsilon = 1e-9) {
    return std::fabs(lhs - rhs) <= epsilon;
}

TerrainRaster make_valid_terrain_raster() {
    TerrainRaster terrain;
    terrain.rows = 2;
    terrain.cols = 3;
    terrain.cell_size_m = 2.0;
    terrain.elevation_m = {
        101.2, 100.7, 100.1,
         99.9,   0.0,  98.8,
    };
    terrain.valid_cell_mask = {
        1, 1, 1,
        1, 0, 1,
    };
    terrain.origin_x_m = 154320.0;
    terrain.origin_y_m = 171205.0;
    terrain.crs_id = "EPSG:31370";
    return terrain;
}

#if FLOODSIM_HAS_GDAL

std::filesystem::path make_temp_raster_path(const std::string& stem) {
    const auto unique_id = std::to_string(std::hash<std::string> {}(stem));
    return std::filesystem::temp_directory_path() / ("floodsim_" + stem + "_" + unique_id + ".tif");
}

void write_test_geotiff(
    const std::filesystem::path& path,
    int rows,
    int cols,
    int bands,
    const std::vector<double>& values,
    const double* geotransform,
    std::optional<double> nodata_value,
    const char* crs_authority = "EPSG",
    int crs_code = 31370) {
    GDALAllRegister();

    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    REQUIRE(driver != nullptr);

    std::unique_ptr<GDALDataset, decltype(&GDALClose)> dataset(
        driver->Create(path.string().c_str(), cols, rows, bands, GDT_Float64, nullptr),
        GDALClose);
    REQUIRE(dataset != nullptr);

    CHECK(dataset->SetGeoTransform(const_cast<double*>(geotransform)) == CE_None);

    OGRSpatialReference spatial_ref;
    spatial_ref.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    CHECK(spatial_ref.SetFromUserInput((std::string(crs_authority) + ":" + std::to_string(crs_code)).c_str()) == OGRERR_NONE);
    char* wkt = nullptr;
    CHECK(spatial_ref.exportToWkt(&wkt) == OGRERR_NONE);
    CHECK(dataset->SetProjection(wkt) == CE_None);
    CPLFree(wkt);

    const std::size_t cells_per_band = static_cast<std::size_t>(rows * cols);
    REQUIRE(values.size() == cells_per_band * static_cast<std::size_t>(bands));

    for (int band_index = 0; band_index < bands; ++band_index) {
        GDALRasterBand* band = dataset->GetRasterBand(band_index + 1);
        REQUIRE(band != nullptr);

        if (nodata_value.has_value()) {
            CHECK(band->SetNoDataValue(*nodata_value) == CE_None);
        }

        const double* band_values = values.data() + (cells_per_band * static_cast<std::size_t>(band_index));
        CHECK(
            band->RasterIO(
                GF_Write,
                0,
                0,
                cols,
                rows,
                const_cast<double*>(band_values),
                cols,
                rows,
                GDT_Float64,
                0,
                0) == CE_None);
    }
}

#endif

}  // namespace

TEST_CASE("rainfall adds water") {
    Grid grid(2, 2);
    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 3600.0);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.012));
    CHECK(nearly_equal(grid.total_water_depth(), 0.048));
}

TEST_CASE("rainfall scales with step duration") {
    Grid grid(1, 1);
    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 1800.0);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.006));
}

TEST_CASE("water flows downhill") {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.5));
}

TEST_CASE("rainfall is applied before flow") {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 0.0);

    RainfallScenario rainfall {1.0};
    SimulationConfig config {
        .time_step_seconds = 3600.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 1.5));
}

TEST_CASE("outflow is split by relative drop") {
    Grid grid(3, 3);
    grid.set_elevation(1, 1, 5.0);
    grid.set_elevation(0, 1, 3.0);
    grid.set_elevation(1, 0, 4.0);
    grid.set_elevation(1, 2, 7.0);
    grid.set_elevation(2, 1, 7.0);
    grid.set_water_depth(1, 1, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.6,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(1, 1), 0.4));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.36));
    CHECK(nearly_equal(grid.water_depth(1, 0), 0.24));
}

TEST_CASE("surface height includes existing water") {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 0.4);
    grid.set_water_depth(0, 1, 1.4);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.4));
    CHECK(nearly_equal(grid.water_depth(0, 1), 1.4));
}

TEST_CASE("flow uses a full grid snapshot") {
    Grid grid(1, 3);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(0, 2, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 2), 0.0));
}

TEST_CASE("repeated steps relay prior inflow on later steps") {
    Grid grid(1, 3);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(0, 2, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);
    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.25));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 2), 0.25));
}

TEST_CASE("closed boundary keeps corner water in domain") {
    Grid grid(2, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 1.0);
    grid.set_elevation(1, 0, 0.0);
    grid.set_elevation(1, 1, 3.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.2));
    CHECK(nearly_equal(grid.water_depth(1, 0), 0.3));
    CHECK(nearly_equal(grid.total_water_depth(), 1.0));
}

TEST_CASE("closed boundary can be selected explicitly in config") {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 2.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_water_depth(0, 0, 1.0);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
        .boundary_mode = BoundaryMode::Closed,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.5));
}

TEST_CASE("closed boundary blocks outflow from edge when no lower in-domain neighbor exists") {
    Grid grid(1, 2);
    grid.set_elevation(0, 0, 5.0);
    grid.set_elevation(0, 1, 6.0);
    grid.set_water_depth(0, 0, 0.8);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.75,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.8));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.0));
    CHECK(nearly_equal(grid.total_water_depth(), 0.8));
}

TEST_CASE("water is conserved without rainfall") {
    Grid grid(3, 3);
    grid.set_elevation(1, 1, 2.0);
    grid.set_elevation(1, 2, 1.0);
    grid.set_elevation(2, 2, 0.0);
    grid.set_water_depth(1, 1, 0.6);
    grid.set_water_depth(0, 0, 0.4);

    const double before = grid.total_water_depth();

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.25,
    };

    for (int i = 0; i < 8; ++i) {
        floodsim::step(grid, rainfall, config);
    }

    CHECK(nearly_equal(before, grid.total_water_depth(), 1e-8));
}

TEST_CASE("repeated steps accumulate rainfall linearly without flow") {
    Grid grid(1, 1);
    RainfallScenario rainfall {0.008};
    SimulationConfig config {
        .time_step_seconds = 900.0,
        .max_outflow_fraction = 0.25,
    };

    for (int i = 0; i < 4; ++i) {
        floodsim::step(grid, rainfall, config);
    }

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.008));
}

TEST_CASE("invalid cells do not receive rainfall") {
    Grid grid(1, 2);
    grid.set_cell_valid(0, 1, false);

    RainfallScenario rainfall {0.012};

    floodsim::add_uniform_rainfall(grid, rainfall, 3600.0);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.012));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.0));
}

TEST_CASE("flow does not route into invalid neighbor cells") {
    Grid grid(1, 3);
    grid.set_elevation(0, 0, 0.0);
    grid.set_elevation(0, 1, 2.0);
    grid.set_elevation(0, 2, 0.0);
    grid.set_water_depth(0, 1, 1.0);
    grid.set_cell_valid(0, 0, false);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.0));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 2), 0.5));
}

TEST_CASE("invalid cells behave like out-of-domain clipped terrain") {
    Grid grid(2, 2);
    grid.set_elevation(0, 0, 5.0);
    grid.set_elevation(0, 1, 0.0);
    grid.set_elevation(1, 0, 1.0);
    grid.set_elevation(1, 1, 2.0);
    grid.set_water_depth(0, 0, 1.0);
    grid.set_cell_valid(0, 1, false);

    RainfallScenario rainfall {};
    SimulationConfig config {
        .time_step_seconds = 1.0,
        .max_outflow_fraction = 0.5,
    };

    floodsim::step(grid, rainfall, config);

    CHECK(nearly_equal(grid.water_depth(0, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(1, 0), 0.5));
    CHECK(nearly_equal(grid.water_depth(0, 1), 0.0));
    CHECK(nearly_equal(grid.total_water_depth(), 1.0));
}

TEST_CASE("csv export writes metadata header and per-cell rows") {
    Grid grid(2, 2);
    grid.set_elevation(0, 0, 1.0);
    grid.set_elevation(0, 1, 1.5);
    grid.set_elevation(1, 0, 2.0);
    grid.set_elevation(1, 1, 2.5);
    grid.set_water_depth(0, 0, 0.25);
    grid.set_water_depth(1, 1, 0.75);

    std::ostringstream output;
    floodsim::write_grid_csv(grid, output);

    const std::string expected =
        "# floodsim_csv_version,1\n"
        "# rows,2\n"
        "# cols,2\n"
        "# cell_size_m,1.000000\n"
        "row,col,elevation_m,water_depth_m,surface_height_m\n"
        "0,0,1.000000,0.250000,1.250000\n"
        "0,1,1.500000,0.000000,1.500000\n"
        "1,0,2.000000,0.000000,2.000000\n"
        "1,1,2.500000,0.750000,3.250000\n";

    CHECK(output.str() == expected);
}

TEST_CASE("csv export writes optional georeferencing metadata when present") {
    Grid grid(1, 1, 2.0);
    grid.set_elevation(0, 0, 100.0);
    grid.set_water_depth(0, 0, 0.5);

    std::ostringstream output;
    floodsim::write_grid_csv(
        grid,
        output,
        floodsim::GridCsvMetadata {
            .origin_x_m = 154320.0,
            .origin_y_m = 171205.0,
            .crs_id = "EPSG:31370",
        });

    const std::string expected =
        "# floodsim_csv_version,1\n"
        "# rows,1\n"
        "# cols,1\n"
        "# cell_size_m,2.000000\n"
        "# origin_x_m,154320.000000\n"
        "# origin_y_m,171205.000000\n"
        "# crs_id,EPSG:31370\n"
        "row,col,elevation_m,water_depth_m,surface_height_m\n"
        "0,0,100.000000,0.500000,100.500000\n";

    CHECK(output.str() == expected);
}

TEST_CASE("valid terrain raster contract passes validation") {
    TerrainRaster terrain = make_valid_terrain_raster();

    floodsim::validate_terrain_raster(terrain);

    CHECK(terrain.cell_count() == 6);
}

TEST_CASE("terrain raster requires matching array sizes") {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.valid_cell_mask.pop_back();

    CHECK_THROWS_AS(floodsim::validate_terrain_raster(terrain), std::invalid_argument);
}

TEST_CASE("terrain raster requires positive cell size") {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.cell_size_m = 0.0;

    CHECK_THROWS_AS(floodsim::validate_terrain_raster(terrain), std::invalid_argument);
}

TEST_CASE("terrain raster requires at least one valid cell") {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.valid_cell_mask = {
        0, 0, 0,
        0, 0, 0,
    };

    CHECK_THROWS_AS(floodsim::validate_terrain_raster(terrain), std::invalid_argument);
}

TEST_CASE("terrain raster requires complete origin metadata") {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.origin_y_m.reset();

    CHECK_THROWS_AS(floodsim::validate_terrain_raster(terrain), std::invalid_argument);
}

TEST_CASE("make_grid_from_terrain preserves geometry and valid mask") {
    const TerrainRaster terrain = make_valid_terrain_raster();

    const Grid grid = floodsim::make_grid_from_terrain(terrain);

    CHECK(grid.rows() == terrain.rows);
    CHECK(grid.cols() == terrain.cols);
    CHECK(nearly_equal(grid.cell_size_m(), terrain.cell_size_m));
    CHECK(nearly_equal(grid.elevation(0, 0), terrain.elevation_m[0]));
    CHECK(grid.is_cell_valid(0, 0));
    CHECK(!grid.is_cell_valid(1, 1));
    CHECK(nearly_equal(grid.water_depth(0, 0), 0.0));
}

TEST_CASE("make_grid_from_terrain rejects invalid contract") {
    TerrainRaster terrain = make_valid_terrain_raster();
    terrain.valid_cell_mask.assign(terrain.cell_count(), 0);

    CHECK_THROWS_AS((void)floodsim::make_grid_from_terrain(terrain), std::invalid_argument);
}

#if FLOODSIM_HAS_GDAL

TEST_CASE("gdal loader reads single-band terrain raster") {
    const std::filesystem::path path = make_temp_raster_path("single_band");
    const double geotransform[6] = {
        154320.0,
        2.0,
        0.0,
        171205.0,
        0.0,
       -2.0,
    };
    const double nodata = -9999.0;

    write_test_geotiff(
        path,
        2,
        3,
        1,
        {
            101.2, 100.7, 100.1,
             99.9, nodata, 98.8,
        },
        geotransform,
        nodata);

    const TerrainRaster terrain = floodsim::load_terrain_raster_from_file(path.string());

    CHECK(terrain.rows == 2);
    CHECK(terrain.cols == 3);
    CHECK(nearly_equal(terrain.cell_size_m, 2.0));
    CHECK(terrain.origin_x_m.has_value());
    CHECK(terrain.origin_y_m.has_value());
    CHECK(terrain.crs_id.has_value());
    CHECK(nearly_equal(*terrain.origin_x_m, 154320.0));
    CHECK(nearly_equal(*terrain.origin_y_m, 171205.0));
    CHECK(*terrain.crs_id == "EPSG:31370");
    CHECK(terrain.elevation_m.size() == 6);
    CHECK(terrain.valid_cell_mask.size() == 6);
    CHECK(terrain.valid_cell_mask[4] == 0);
    CHECK(terrain.valid_cell_mask[0] == 1);
    CHECK(terrain.valid_cell_mask[5] == 1);

    std::filesystem::remove(path);
}

TEST_CASE("gdal loader rejects multi-band rasters") {
    const std::filesystem::path path = make_temp_raster_path("multi_band");
    const double geotransform[6] = {
        0.0,
        1.0,
        0.0,
        0.0,
        0.0,
       -1.0,
    };

    write_test_geotiff(
        path,
        1,
        2,
        2,
        {
            1.0, 2.0,
            3.0, 4.0,
        },
        geotransform,
        std::nullopt);

    CHECK_THROWS_AS((void)floodsim::load_terrain_raster_from_file(path.string()), std::invalid_argument);

    std::filesystem::remove(path);
}

TEST_CASE("gdal loader rejects non-square pixels") {
    const std::filesystem::path path = make_temp_raster_path("non_square");
    const double geotransform[6] = {
        0.0,
        2.0,
        0.0,
        0.0,
        0.0,
       -3.0,
    };

    write_test_geotiff(
        path,
        1,
        2,
        1,
        {
            1.0, 2.0,
        },
        geotransform,
        std::nullopt);

    CHECK_THROWS_AS((void)floodsim::load_terrain_raster_from_file(path.string()), std::invalid_argument);

    std::filesystem::remove(path);
}

#endif
