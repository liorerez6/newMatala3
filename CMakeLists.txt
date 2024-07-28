cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(BlockchainMiner VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the shared library
add_library(SharedFile SharedFile.cpp)

# Link against zlib for the shared library
find_package(ZLIB REQUIRED)
target_link_libraries(SharedFile ZLIB::ZLIB)

# Add executables
add_executable(newMiner newMiner.cpp)
add_executable(newServer newServer.cpp)

# Link the shared library to the executables
target_link_libraries(newMiner SharedFile)
target_link_libraries(newServer SharedFile)