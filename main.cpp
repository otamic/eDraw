#include <iostream>
#include <cstdlib>

#include "m_driver.h"

int main(const int argc, const char **argv) {
    if (argc == 2) {
        // for global variable
        SymbolManager::addLayer();
        SymbolManager::addStatus(SymbolManager::MAIN);
        SymbolManager::addLayerF();

        MC::Driver driver;
        if (std::strncmp(argv[1], "-o", 2) == 0)
            driver.parse(std::cin);
        else {
            driver.parse(argv[1]);
        }
    }
    else
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}