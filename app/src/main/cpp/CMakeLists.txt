cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

set(CMAKE_CXX_STANDARD 17)

# تحديد مسار OpenSSL المرفوع في المستودع
set(OPENSSL_DIR ${CMAKE_SOURCE_DIR}/openssl/${ANDROID_ABI})

# إضافة مسار الـ include
include_directories(${OPENSSL_DIR}/include)

# إضافة مكتبات OpenSSL
add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION
    ${OPENSSL_DIR}/lib/libssl.a)

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION
    ${OPENSSL_DIR}/lib/libcrypto.a)

# المكتبة الرئيسية
add_library(looka_engine SHARED
    looka_engine.cpp
)

# مكتبة log
find_library(log-lib log)

# ربط المكتبات
target_link_libraries(looka_engine
    ssl
    crypto
    ${log-lib}
)
