cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

# 1. تحديد المسار المطلق لمجلد العمل
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 2. بناء المحرك
add_library(looka_engine SHARED looka_engine.cpp)

# 3. إجبار المترجم على رؤية مجلدات OpenSSL (هذا هو الحل)
target_include_directories(looka_engine PRIVATE 
    ${SOURCE_DIR}/openssl/${ANDROID_ABI}/include
    ${SOURCE_DIR}/openssl/${ANDROID_ABI}/include/openssl
)

# 4. تعريف المكتبات الجاهزة (.a)
add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION 
    ${SOURCE_DIR}/openssl/${ANDROID_ABI}/lib/libcrypto.a)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION 
    ${SOURCE_DIR}/openssl/${ANDROID_ABI}/lib/libssl.a)

# 5. ربط المكتبات
target_link_libraries(looka_engine
    crypto
    ssl
    log
    android
    z
)
