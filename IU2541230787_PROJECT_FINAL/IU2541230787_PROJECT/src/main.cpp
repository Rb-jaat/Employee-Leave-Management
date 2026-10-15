// main.cpp - creates the system object, loads data, shows the menu. No business logic here.
#include <cstring>
#include <iostream>
#include "elm/ConsoleUI.h"
#include "elm/errors.h"

int main(int argc, char** argv) {
    std::string dir = "Data";
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--data") == 0) dir = argv[i + 1];
    try {
        elm::LeaveSystem system(dir);
        elm::ConsoleUI ui(system);
        return ui.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
