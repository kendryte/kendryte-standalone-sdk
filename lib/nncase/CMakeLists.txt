add_subdirectory(v0)
add_subdirectory(v1)

add_library(nncase-wrapper STATIC nncase.cpp)
target_link_libraries(nncase-wrapper PRIVATE nncase-v0 nncase-v1)
target_include_directories(nncase-wrapper PUBLIC include)
