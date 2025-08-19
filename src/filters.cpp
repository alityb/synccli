#include "filters.hpp"

#include <sstream>

namespace {

bool regexMatchAny(const std::vector<std::regex>& regexes, const std::string& text) {
    for (const auto& r : regexes) {
        if (std::regex_match(text, r)) return true;
    }
    return false;
}

std::string escapeRegex(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    const std::string special = R"(.^$|()[]{}+\\)";
    for (char c : s) {
        if (special.find(c) != std::string::npos) {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

}

std::string globToRegexPattern(const std::string& globPattern) {
    // Convert a simple glob to regex: * -> .*, ? -> .
    // If pattern ends with '/', treat as directory prefix (append '.*').
    bool dirPrefix = !globPattern.empty() && globPattern.back() == '/';
    std::string pattern = globPattern;
    if (dirPrefix) {
        pattern.pop_back();
    }

    std::ostringstream oss;
    oss << '^';
    for (size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        if (c == '*') {
            oss << ".*";
        } else if (c == '?') {
            oss << '.';
        } else {
            // Escape regex special characters
            const std::string special = R"(.^$|()[]{}+\\)";
            if (special.find(c) != std::string::npos) {
                oss << '\\';
            }
            oss << c;
        }
    }
    if (dirPrefix) {
        oss << "/.*";
    }
    oss << '$';
    return oss.str();
}

PathFilter::PathFilter() = default;

void PathFilter::setIncludePatterns(const std::vector<std::string>& includeGlobs) {
    includeRegexes.clear();
    hasInclude = !includeGlobs.empty();
    for (const auto& g : includeGlobs) {
        includeRegexes.emplace_back(globToRegexPattern(g));
    }
}

void PathFilter::setExcludePatterns(const std::vector<std::string>& excludeGlobs) {
    excludeRegexes.clear();
    for (const auto& g : excludeGlobs) {
        excludeRegexes.emplace_back(globToRegexPattern(g));
    }
}

bool PathFilter::shouldInclude(const std::string& relativePath) const {
    // Normalize input: ensure it uses '/' separators (caller should do this).
    if (hasInclude && !regexMatchAny(includeRegexes, relativePath)) {
        return false;
    }
    if (regexMatchAny(excludeRegexes, relativePath)) {
        return false;
    }
    return true;
}
