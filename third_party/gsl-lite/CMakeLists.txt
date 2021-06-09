cmake_minimum_required (VERSION 3.8)

add_library(gsl-lite INTERFACE)
target_include_directories(gsl-lite INTERFACE include)
add_library(CONAN_PKG::gsl-lite ALIAS gsl-lite)
