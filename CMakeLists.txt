cmake_minimum_required (VERSION 2.6)
project (low-level-string-pool)

# If you have access to google::sparse_hash_map, it is a huge win in terms of memory.
# See https://github.com/sparsehash/sparsehash
set(CMAKE_CXX_FLAGS "-I . -I src -g -Wall -std=c++14 -DLLMP_HAS_SPARSE_MAP -O3 -DNDEBUG")
#set(CMAKE_CXX_FLAGS "-I . -I src -g -Wall -std=c++14 -O3 -DNDEBUG")

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/lib")

add_executable(driver example/driver.cpp src/string_pool.cpp)

# "catch" based unit tests
enable_testing()
add_executable(all-catch-tests test/catch_main.cpp test/test_string_pool.cpp src/string_pool.cpp)

add_test(all-catch-tests all-catch-tests)
