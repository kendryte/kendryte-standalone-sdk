cmake_minimum_required (VERSION 3.8)

add_library(mpark-variant INTERFACE)
target_include_directories(mpark-variant INTERFACE include)
add_library(CONAN_PKG::mpark-variant ALIAS mpark-variant)
