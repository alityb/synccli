#pragma once

#include <filesystem>
#include <iostream>
#include <string>

namespace utils {

std::string toGenericString(const std::filesystem::path& p);

// Ensure parent directory for a file exists. Returns true on success or dry-run.
bool ensureParentDirectory(const std::filesystem::path& filePath, bool dryRun, std::ostream& out, std::ostream& err);

// Compare source and destination files. Returns true if they differ (size or timestamp).
bool filesDiffer(const std::filesystem::path& src, const std::filesystem::path& dst);

// Join and normalize a relative path using POSIX separators.
std::string makeRelativePOSIX(const std::filesystem::path& base, const std::filesystem::path& p);

}
