# SharedLibrary/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(SharedFile)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find ZLIB
find_package(ZLIB REQUIRED)

# Add the shared library
add_library(SharedFile SHARED SharedFile.cpp)

# Link against zlib
target_link_libraries(SharedFile ZLIB::ZLIB)