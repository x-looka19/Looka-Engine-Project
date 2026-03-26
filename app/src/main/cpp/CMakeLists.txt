cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

set(CMAKE_CXX_STANDARD 17)

set(OPENSSL_DIR ${CMAKE_SOURCE_DIR}/openssl/${ANDROID_ABI})

file(GLOB_RECURSE CONF_PATH "${OPENSSL_DIR}/include/*/opensslconf.h")
if(CONF_PATH)
    list(GET CONF_PATH 0 FIRST_CONF_PATH)
    get_filename_component(CONF_DIR ${FIRST_CONF_PATH} DIRECTORY)
    include_directories(${CONF_DIR})
endif()

include_directories(${OPENSSL_DIR}/include)
include_directories(${OPENSSL_DIR}/include/openssl)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${OPENSSL_DIR}/lib/libssl.a)

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_DIR}/lib/libcrypto.a)

add_library(looka_engine SHARED looka_engine.cpp)

find_library(log-lib log)

target_link_libraries(looka_engine ssl crypto ${log-lib})
