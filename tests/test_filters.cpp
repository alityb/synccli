#include <iostream>
#include <string>

#include "filters.hpp"

static int failures_filters = 0;

static void expectTrueF(bool cond, const std::string& msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << std::endl;
        ++failures_filters;
    }
}

static void expectFalseF(bool cond, const std::string& msg) {
    expectTrueF(!cond, msg);
}

int run_test_filters() {
    std::cout << "[RUN] filters" << std::endl;

    // Test glob to regex basics indirectly via filter
    PathFilter f1;
    f1.setIncludePatterns({});
    f1.setExcludePatterns({"*.log", "node_modules/"});

    expectTrueF(f1.shouldInclude("src/main.cpp"), "include normal file");
    expectFalseF(f1.shouldInclude("debug.log"), "exclude *.log");
    expectFalseF(f1.shouldInclude("node_modules/lib/index.js"), "exclude dir prefix");

    PathFilter f2;
    f2.setIncludePatterns({"*.txt"});
    f2.setExcludePatterns({"secret*"});

    expectTrueF(f2.shouldInclude("a/readme.txt"), "include *.txt");
    expectFalseF(f2.shouldInclude("a/image.png"), "exclude not included by include-rule");
    expectFalseF(f2.shouldInclude("secret.txt"), "exclude wins after include");

    std::cout << "[DONE] filters" << std::endl;
    return failures_filters;
}
