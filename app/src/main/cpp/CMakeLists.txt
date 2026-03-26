cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(src/main/cpp/include)

# Source files
add_library(looka_engine SHARED
    src/main/cpp/looka_engine.cpp
)

# Find required libraries
find_library(log-lib log)
find_library(android-lib android)

# Link libraries
target_link_libraries(looka_engine
    ${log-lib}
    ${android-lib}
)

# Set target properties
set_target_properties(looka_engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ANDROID_ABI}"
)