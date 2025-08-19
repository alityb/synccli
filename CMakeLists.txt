cmake_minimum_required(VERSION 3.12)
project(synccli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(synccore
    src/cli.cpp
    src/sync.cpp
    src/filters.cpp
    src/utils.cpp
)

target_include_directories(synccore PUBLIC include)

target_compile_options(synccore PRIVATE -Wall -Wextra -Wpedantic)

add_executable(synccli src/main.cpp)

target_link_libraries(synccli PRIVATE synccore)

target_compile_options(synccli PRIVATE -Wall -Wextra -Wpedantic)

add_executable(synccli_tests
    tests/test_main.cpp
    tests/test_utils.cpp
    tests/test_filters.cpp
    tests/test_sync.cpp
)

target_link_libraries(synccli_tests PRIVATE synccore)

target_compile_options(synccli_tests PRIVATE -Wall -Wextra -Wpedantic) 