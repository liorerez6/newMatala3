cmake_minimum_required(VERSION 3.10)

# Project name
project(ServerProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES newServer.cpp SharedFile.cpp)

# Include directories
include_directories(.)

# Find and link zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# Add the executable
add_executable(newServer ${SOURCES})

# Link libraries
target_link_libraries(newServer ${ZLIB_LIBRARIES} pthread)