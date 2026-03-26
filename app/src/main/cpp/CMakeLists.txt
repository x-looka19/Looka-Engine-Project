cmake_minimum_required(VERSION 3.22.1)
project("looka_engine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# إضافة المكتبة الرئيسية
add_library(looka_engine SHARED
    looka_engine.cpp
)

# مكتبة log
find_library(log-lib log)

# OpenSSL - إذا كان موجوداً
# إذا لم يكن OpenSSL مثبتاً، علق الأسطر التالية
# find_library(ssl-lib ssl)
# find_library(crypto-lib crypto)
# target_link_libraries(looka_engine ${ssl-lib} ${crypto-lib})

# ربط المكتبات
target_link_libraries(looka_engine
    ${log-lib}
)
