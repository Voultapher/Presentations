cmake_minimum_required (VERSION 3.8)
project (beast VERSION 0.1 LANGUAGES CXX)

find_package (Threads REQUIRED)
find_package (Boost 1.64.0 REQUIRED COMPONENTS system)

add_subdirectory(src)
add_subdirectory(test)
