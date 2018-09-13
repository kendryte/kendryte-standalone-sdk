if (NOT BUILDING_SDK)
    add_library(kendryte STATIC IMPORTED)
    set_property(TARGET kendryte PROPERTY IMPORTED_LOCATION ${SDK_ROOT}/libmaix.a)
    include_directories(${SDK_ROOT}/include/)
endif ()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
# add_dependencies(${PROJECT_NAME} kendryte) # TODO: third_party
# target_link_libraries(${PROJECT_NAME} kendryte) # TODO: third_party
# link_directories(${CMAKE_BINARY_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(${PROJECT_NAME}
        -Wl,--start-group
        gcc m c kendryte
        -Wl,--end-group
        )

IF(SUFFIX)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX ${SUFFIX})
ENDIF()

#message("CMAKE_OBJCOPY=${CMAKE_OBJCOPY}")

# Build target
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${SUFFIX} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating .bin file ...")


add_custom_target(firmware DEPENDS ${PROJECT_NAME}.firmware.bin)

# show information
include(${CMAKE_CURRENT_LIST_DIR}/dump-config.cmake)
