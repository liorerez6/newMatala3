cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(BlockchainMiner VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add an executable
add_executable(main main.cpp)

# Link against zlib
find_package(ZLIB REQUIRED)
target_link_libraries(main ZLIB::ZLIB)