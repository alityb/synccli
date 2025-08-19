#include "cli.hpp"

#include <algorithm>

namespace {

bool consumeOptionWithValue(int& i, int argc, char** argv, std::string& outValue) {
    if (i + 1 >= argc) return false;
    outValue = argv[++i];
    return true;
}

}

void printUsage(std::ostream& out) {
    out << "synccli - A simple file synchronization tool\n";
    out << "\n";
    out << "Usage:\n";
    out << "  synccli -s <source> -d <destination> [--dry-run] [--mirror]\n";
    out << "          [--exclude <pattern>]... [--include <pattern>]... [--time]\n";
    out << "\n";
    out << "Options:\n";
    out << "  -s, --source <path>        Source directory\n";
    out << "  -d, --destination <path>   Destination directory\n";
    out << "      --dry-run              Show what would happen without making changes\n";
    out << "      --mirror               Delete files in destination not present in source (respecting filters)\n";
    out << "      --exclude <pattern>    Glob pattern to exclude (can be repeated)\n";
    out << "      --include <pattern>    Glob pattern to include (can be repeated). If any includes are set, only\n";
    out << "                             matching paths are considered (before applying excludes).\n";
    out << "      --time                 Print timing and throughput summary\n";
    out << "      --help                 Show this help\n";
}

bool parseCLI(int argc, char** argv, CLIOptions& options, std::ostream& out, std::ostream& err) {
    if (argc <= 1) {
        printUsage(out);
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s" || arg == "--source") {
            std::string value;
            if (!consumeOptionWithValue(i, argc, argv, value)) {
                err << "Missing value for " << arg << "\n";
                return false;
            }
            options.sourcePath = value;
        } else if (arg == "-d" || arg == "--destination") {
            std::string value;
            if (!consumeOptionWithValue(i, argc, argv, value)) {
                err << "Missing value for " << arg << "\n";
                return false;
            }
            options.destinationPath = value;
        } else if (arg == "--dry-run") {
            options.dryRun = true;
        } else if (arg == "--mirror") {
            options.mirror = true;
        } else if (arg == "--time") {
            options.showTime = true;
        } else if (arg == "--exclude") {
            std::string value;
            if (!consumeOptionWithValue(i, argc, argv, value)) {
                err << "Missing value for --exclude\n";
                return false;
            }
            options.excludePatterns.push_back(value);
        } else if (arg == "--include") {
            std::string value;
            if (!consumeOptionWithValue(i, argc, argv, value)) {
                err << "Missing value for --include\n";
                return false;
            }
            options.includePatterns.push_back(value);
        } else if (arg == "--help" || arg == "-h") {
            printUsage(out);
            return false;
        } else {
            err << "Unknown option: " << arg << "\n";
            return false;
        }
    }

    if (options.sourcePath.empty() || options.destinationPath.empty()) {
        err << "Both --source and --destination must be provided.\n";
        return false;
    }

    return true;
}
