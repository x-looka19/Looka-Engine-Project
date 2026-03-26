cmake_minimum_required(VERSION 3.22.1)

project("looka_engine")

set(OPENSSL_DIR ${CMAKE_SOURCE_DIR}/openssl/${ANDROID_ABI})

include_directories(${OPENSSL_DIR}/include)
include_directories(${OPENSSL_DIR}/include/openssl)

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION 
    ${OPENSSL_DIR}/lib/libcrypto.a)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION 
    ${OPENSSL_DIR}/lib/libssl.a)

add_library(looka_engine SHARED looka_engine.cpp)

target_link_libraries(looka_engine
    crypto
    ssl
    log
    android
    z)
