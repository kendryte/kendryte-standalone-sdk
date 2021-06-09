cmake_minimum_required (VERSION 3.8)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE include)
add_library(CONAN_PKG::nlohmann_json ALIAS nlohmann_json)