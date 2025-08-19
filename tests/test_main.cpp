#include <iostream>

int run_test_utils();
int run_test_filters();
int run_test_sync();

int main() {
    int failures = 0;
    failures += run_test_utils();
    failures += run_test_filters();
    failures += run_test_sync();

    if (failures == 0) {
        std::cout << "All tests passed" << std::endl;
        return 0;
    }
    std::cout << failures << " test(s) failed" << std::endl;
    return 1;
} 