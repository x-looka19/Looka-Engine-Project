cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

add_library(looka_engine SHARED
    looka_engine.cpp
)

find_library(log-lib log)
target_link_libraries(looka_engine ${log-lib})
