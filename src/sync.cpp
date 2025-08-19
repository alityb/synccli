#include "sync.hpp"

#include "utils.hpp"

#include <filesystem>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

namespace {

bool shouldCopyOrOverwrite(const fs::path& src, const fs::path& dst, bool& isOverwrite) {
    isOverwrite = fs::exists(dst);
    return utils::filesDiffer(src, dst);
}

}

int runSync(const CLIOptions& options, std::ostream& out, std::ostream& err) {
    const fs::path& srcRoot = options.sourcePath;
    const fs::path& dstRoot = options.destinationPath;

    if (!fs::exists(srcRoot) || !fs::is_directory(srcRoot)) {
        err << "Source path does not exist or is not a directory: " << utils::toGenericString(srcRoot) << "\n";
        return 1;
    }

    auto t0 = std::chrono::steady_clock::now();

    // Prepare filter
    PathFilter filter;
    filter.setIncludePatterns(options.includePatterns);
    filter.setExcludePatterns(options.excludePatterns);

    SyncStats stats;

    // Gather all included source files and perform copy/overwrite
    std::unordered_set<std::string> includedSourceFiles;

    std::error_code ec;
    for (fs::recursive_directory_iterator it(srcRoot, ec), end; it != end; it.increment(ec)) {
        if (ec) {
            err << "Traversal error: " << ec.message() << "\n";
            return 1;
        }
        const fs::directory_entry& entry = *it;
        if (entry.is_directory()) {
            continue;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string rel = utils::makeRelativePOSIX(srcRoot, entry.path());
        if (!filter.shouldInclude(rel)) {
            ++stats.filesSkipped;
            continue;
        }
        includedSourceFiles.insert(rel);

        fs::path dstPath = dstRoot / fs::path(rel);

        bool isOverwrite = false;
        bool needsCopy = shouldCopyOrOverwrite(entry.path(), dstPath, isOverwrite);
        if (needsCopy) {
            // Count bytes even for dry-run to estimate throughput
            std::error_code sizeEc;
            auto sz = fs::file_size(entry.path(), sizeEc);
            if (!sizeEc) {
                stats.bytesTransferred += sz;
            }

            if (!utils::ensureParentDirectory(dstPath, options.dryRun, out, err)) {
                return 1;
            }
            if (options.dryRun) {
                if (isOverwrite) {
                    out << "[DRY RUN] Would overwrite: " << utils::toGenericString(entry.path())
                        << " \u2192 " << utils::toGenericString(dstPath) << "\n";
                } else {
                    out << "[DRY RUN] Would copy: " << utils::toGenericString(entry.path())
                        << " \u2192 " << utils::toGenericString(dstPath) << "\n";
                }
            } else {
                std::error_code cpEc;
                fs::copy_file(entry.path(), dstPath, fs::copy_options::overwrite_existing, cpEc);
                if (cpEc) {
                    err << "Copy failed '" << utils::toGenericString(entry.path()) << "' -> '"
                        << utils::toGenericString(dstPath) << "': " << cpEc.message() << "\n";
                    return 1;
                }
                // Attempt to preserve timestamp from source
                auto srcTime = fs::last_write_time(entry.path(), cpEc);
                if (!cpEc) {
                    fs::last_write_time(dstPath, srcTime, cpEc);
                }
            }
            if (isOverwrite) ++stats.filesOverwritten; else ++stats.filesCopied;
        } else {
            ++stats.filesSkipped;
        }
    }

    // Mirror mode: delete files in destination that are not present in source included set
    if (options.mirror) {
        for (fs::recursive_directory_iterator it(dstRoot, ec), end; it != end; it.increment(ec)) {
            if (ec) {
                err << "Traversal error: " << ec.message() << "\n";
                return 1;
            }
            const fs::directory_entry& entry = *it;
            if (!entry.is_regular_file()) continue;
            std::string rel = utils::makeRelativePOSIX(dstRoot, entry.path());
            // Only consider deleting files that would be included by the filter
            if (!filter.shouldInclude(rel)) continue;
            if (includedSourceFiles.find(rel) == includedSourceFiles.end()) {
                if (options.dryRun) {
                    out << "[DRY RUN] Would delete: " << utils::toGenericString(entry.path()) << "\n";
                } else {
                    std::error_code rmEc;
                    fs::remove(entry.path(), rmEc);
                    if (rmEc) {
                        err << "Delete failed '" << utils::toGenericString(entry.path()) << "': " << rmEc.message() << "\n";
                        return 1;
                    }
                }
                ++stats.filesDeleted;
            }
        }
    }

    // Summary
    if (options.dryRun) {
        out << "[SUMMARY] " << stats.filesCopied << " files would be copied, "
            << stats.filesOverwritten << " files would be overwritten, "
            << stats.filesDeleted << " files would be deleted." << "\n";
    } else {
        out << "[SUMMARY] Copied: " << stats.filesCopied
            << ", Overwritten: " << stats.filesOverwritten
            << ", Deleted: " << stats.filesDeleted
            << ", Skipped: " << stats.filesSkipped << "\n";
    }

    auto t1 = std::chrono::steady_clock::now();
    if (options.showTime) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        double seconds = ms / 1000.0;
        double mib = static_cast<double>(stats.bytesTransferred) / (1024.0 * 1024.0);
        double mibps = seconds > 0.0 ? (mib / seconds) : 0.0;
        std::ios::fmtflags f(out.flags());
        out << "[TIMING] Duration: " << ms << " ms, Transferred: " << std::fixed << std::setprecision(2)
            << mib << " MiB, Throughput: " << mibps << " MiB/s\n";
        out.flags(f);
    }

    return 0;
}