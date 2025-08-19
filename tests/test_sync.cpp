#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#include "cli.hpp"
#include "sync.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;

static int failures_sync = 0;

static void expectTrueS(bool cond, const std::string& msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << std::endl;
        ++failures_sync;
    }
}

static void writeFile(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream ofs(p); ofs << content; ofs.close();
}

static std::string tmpBase() {
    auto base = fs::temp_directory_path() / ("synccli_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(base);
    return base.string();
}

int run_test_sync() {
    std::cout << "[RUN] sync" << std::endl;

    fs::path base = fs::path(tmpBase());
    fs::path src = base / "src";
    fs::path dst = base / "dst";

    // Prepare source files
    writeFile(src / "a.txt", "hello");
    writeFile(src / "b/b.txt", "world");
    writeFile(src / "logs/app.log", "log");

    // Existing destination file to be overwritten
    writeFile(dst / "a.txt", "old");

    // Mirror-only stale file
    writeFile(dst / "stale.txt", "stale");

    // Dry-run without mirror
    {
        CLIOptions opts;
        opts.sourcePath = src;
        opts.destinationPath = dst;
        opts.dryRun = true;
        opts.mirror = false;
        opts.excludePatterns = {"*.log"};
        int rc = runSync(opts, std::cout, std::cerr);
        expectTrueS(rc == 0, "dry-run sync rc==0");
        // No changes should be applied in dry run
        expectTrueS(fs::exists(dst / "stale.txt"), "dry-run didn't delete stale");
    }

    // Real sync with mirror
    {
        CLIOptions opts;
        opts.sourcePath = src;
        opts.destinationPath = dst;
        opts.dryRun = false;
        opts.mirror = true;
        opts.excludePatterns = {"*.log"};
        int rc = runSync(opts, std::cout, std::cerr);
        expectTrueS(rc == 0, "real sync rc==0");
        expectTrueS(fs::exists(dst / "a.txt"), "a.txt exists");
        expectTrueS(fs::exists(dst / "b/b.txt"), "b/b.txt exists");
        expectTrueS(!fs::exists(dst / "logs/app.log"), "excluded file not copied");
        expectTrueS(!fs::exists(dst / "stale.txt"), "mirror deleted stale");
    }

    // Cleanup
    std::error_code ec;
    fs::remove_all(base, ec);

    std::cout << "[DONE] sync" << std::endl;
    return failures_sync;
}
