
set(nncaseruntime_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/cmake/nncaseruntime)
find_package(nncaseruntime REQUIRED)

add_library(nncase-v1 STATIC nncase_v1.cpp)
target_link_libraries(nncase-v1 PRIVATE -Wl,-start-group nncaseruntime nncase_rt_modules_k210 -Wl,-end-group)