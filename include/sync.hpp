#pragma once

#include <iostream>
#include <string>
#include <unordered_set>
#include <cstdint>

#include "cli.hpp"
#include "filters.hpp"

struct SyncStats {
    std::size_t filesCopied = 0;
    std::size_t filesOverwritten = 0;
    std::size_t filesDeleted = 0;
    std::size_t filesSkipped = 0;
    std::uintmax_t bytesTransferred = 0;
};

// Execute synchronization according to options.
// Returns 0 on success, non-zero on error.
int runSync(const CLIOptions& options, std::ostream& out, std::ostream& err);