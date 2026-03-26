cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

# إضافة مسار OpenSSL (سيبحث عنه النظام)
set(CMAKE_CXX_STANDARD 17)

# البحث عن OpenSSL
find_package(OpenSSL REQUIRED)

# إضافة المكتبة
add_library(looka_engine SHARED
    looka_engine.cpp
)

# ربط المكتبات
target_link_libraries(looka_engine
    ${log-lib}
    ${OPENSSL_LIBRARIES}
)

# إضافة مسارات الـ include
target_include_directories(looka_engine PRIVATE
    ${OPENSSL_INCLUDE_DIR}
)

# مكتبة log
find_library(log-lib log)
