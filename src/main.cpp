#include "cli.hpp"
#include "sync.hpp"

#include <iostream>

int main(int argc, char** argv) {
    CLIOptions options;
    if (!parseCLI(argc, argv, options, std::cout, std::cerr)) {
        // parseCLI already printed usage or error
        return 1;
    }
    return runSync(options, std::cout, std::cerr);
}
