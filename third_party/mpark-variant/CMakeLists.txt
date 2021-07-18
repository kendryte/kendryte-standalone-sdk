cmake_minimum_required (VERSION 3.8)

add_library(mpark_variant INTERFACE)
target_include_directories(mpark_variant INTERFACE include)
add_library(mpark_variant::mpark_variant ALIAS mpark_variant)
add_library(CONAN_PKG::mpark-variant ALIAS mpark_variant)
