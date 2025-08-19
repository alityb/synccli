#include <iostream>
#include <filesystem>
#include <string>

#include "utils.hpp"

static int failures = 0;

static void expectTrue(bool cond, const std::string& msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << std::endl;
        ++failures;
    }
}

static void expectEq(const std::string& a, const std::string& b, const std::string& msg) {
    if (a != b) {
        std::cout << "[FAIL] " << msg << " | expected '" << b << "' got '" << a << "'" << std::endl;
        ++failures;
    }
}

int run_test_utils() {
    namespace fs = std::filesystem;
    std::cout << "[RUN] utils" << std::endl;

    fs::path base = fs::current_path();
    fs::path file = base / "a" / "b" / "c.txt";
    std::string rel = utils::makeRelativePOSIX(base, file);
    expectEq(rel, "a/b/c.txt", "makeRelativePOSIX relative path");

    std::cout << "[DONE] utils" << std::endl;
    return failures;
}
