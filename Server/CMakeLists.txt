# Server/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Server)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find ZLIB
find_package(ZLIB REQUIRED)

# Add the shared library directorys
add_subdirectory(/app/SharedLibrary ${CMAKE_BINARY_DIR}/app)

# Add the executable
add_executable(newServer newServer.cpp)

# Link the shared library and ZLIB to the executable
target_link_libraries(newServer SharedFile ZLIB::ZLIB)
