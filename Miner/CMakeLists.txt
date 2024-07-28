# miner/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Miner)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find ZLIB
find_package(ZLIB REQUIRED)

# Add the executable
add_executable(newMiner newMiner.cpp)

# Link the shared library and ZLIB to the executable
target_link_libraries(newMiner /app/SharedLibrary/build/libSharedFile.so ZLIB::ZLIB)