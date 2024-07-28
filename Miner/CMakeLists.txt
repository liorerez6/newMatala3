# miner/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Project name
project(MinerProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES newMiner.cpp SharedFile.cpp)

# Include directories
include_directories(.)

# Find and link zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# Add the executable
add_executable(newMiner ${SOURCES})

# Link libraries
target_link_libraries(newMiner ${ZLIB_LIBRARIES} pthread)