
#include "App.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

