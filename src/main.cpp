
#include "App.hpp"
#include "ConfigParser.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h> // for getcwd
#include <limits.h> // for PATH_MAX

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file.scop>" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        AppConfig config = ConfigParser::parse(argv[1]);
        App app(config);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

