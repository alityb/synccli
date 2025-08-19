#pragma once

#include <regex>
#include <string>
#include <vector>

// Converts a glob-like pattern (supports * and ?) to a std::regex string.
// Examples:
//  "*.log" -> "^.*\\.log$"
//  "node_modules/" -> "^node_modules/.*$"
std::string globToRegexPattern(const std::string& globPattern);

class PathFilter {
public:
    PathFilter();

    // Patterns are glob-like; they will be compiled to regex internally.
    void setIncludePatterns(const std::vector<std::string>& includeGlobs);
    void setExcludePatterns(const std::vector<std::string>& excludeGlobs);

    // Returns true if the given relative path should be included.
    // The path should be a POSIX-style relative path (use '/' separators).
    bool shouldInclude(const std::string& relativePath) const;

    bool hasIncludeRules() const { return hasInclude; }

private:
    std::vector<std::regex> includeRegexes;
    std::vector<std::regex> excludeRegexes;
    bool hasInclude = false;
};
