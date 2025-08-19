#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

struct CLIOptions {
    std::filesystem::path sourcePath;
    std::filesystem::path destinationPath;
    bool dryRun = false;
    bool mirror = false;
    bool showTime = false;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> includePatterns;
};

// Parses command-line arguments into CLIOptions.
// Returns true on success, false if there was a parsing/validation error.
bool parseCLI(int argc, char** argv, CLIOptions& options, std::ostream& out, std::ostream& err);

// Writes a brief usage/help message to the provided stream.
void printUsage(std::ostream& out);
