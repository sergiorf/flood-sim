#include "example_runner.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        const auto arguments = floodsim::examples::real_terrain::parse_arguments(argc, argv);
        return floodsim::examples::real_terrain::execute_example_cli(
            arguments,
            std::cout,
            std::cerr);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
