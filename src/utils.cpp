#include "utils.hpp"

#include <chrono>

namespace utils {

std::string toGenericString(const std::filesystem::path& p) {
    return p.generic_string();
}

bool ensureParentDirectory(const std::filesystem::path& filePath, bool dryRun, std::ostream& out, std::ostream& err) {
    std::filesystem::path parent = filePath.parent_path();
    if (parent.empty()) return true;
    if (std::filesystem::exists(parent)) return true;
    if (dryRun) {
        out << "[DRY RUN] Would create directory: " << toGenericString(parent) << "\n";
        return true;
    }
    std::error_code ec;
    if (!std::filesystem::create_directories(parent, ec)) {
        if (ec) {
            err << "Failed to create directory '" << toGenericString(parent) << "': " << ec.message() << "\n";
            return false;
        }
    }
    return true;
}

bool filesDiffer(const std::filesystem::path& src, const std::filesystem::path& dst) {
    std::error_code ec;
    if (!std::filesystem::exists(dst)) return true;
    auto srcStatus = std::filesystem::status(src, ec);
    if (ec) return true;
    auto dstStatus = std::filesystem::status(dst, ec);
    if (ec) return true;
    if (!std::filesystem::is_regular_file(srcStatus) || !std::filesystem::is_regular_file(dstStatus)) return true;

    auto srcSize = std::filesystem::file_size(src, ec);
    if (ec) return true;
    auto dstSize = std::filesystem::file_size(dst, ec);
    if (ec) return true;
    if (srcSize != dstSize) return true;

    auto srcTime = std::filesystem::last_write_time(src, ec);
    if (ec) return true;
    auto dstTime = std::filesystem::last_write_time(dst, ec);
    if (ec) return true;

    return srcTime != dstTime;
}

std::string makeRelativePOSIX(const std::filesystem::path& base, const std::filesystem::path& p) {
    std::error_code ec;
    auto rel = std::filesystem::relative(p, base, ec);
    if (ec) {
        // Fallback: strip common prefix manually
        auto absBase = std::filesystem::weakly_canonical(base, ec);
        auto absP = std::filesystem::weakly_canonical(p, ec);
        if (!ec) {
            auto sBase = absBase.generic_string();
            auto sP = absP.generic_string();
            if (sP.rfind(sBase + '/', 0) == 0) {
                return sP.substr(sBase.size() + 1);
            }
        }
        return p.filename().generic_string();
    }
    return rel.generic_string();
}

}
