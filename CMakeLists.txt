# [NEVER REMOVE THIS LINE] WARNING: this file is generated, please edit kendryte-package.json file instead.
##### include(reset) #####
### reset basic var
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# set(CMAKE_SYSTEM "Generic")
set(CMAKE_SYSTEM_NAME "Generic")

# set(CMAKE_HOST_SYSTEM "Generic")
# set(CMAKE_HOST_SYSTEM_NAME "Generic")
#
set(CMAKE_CROSSCOMPILING TRUE)
# set(CMAKE_C_COMPILER_FORCED TRUE)
# set(CMAKE_CXX_COMPILER_FORCED TRUE)
# set(CMAKE_ASM_COMPILER_FORCED TRUE)
#

set(SOURCE_FILES "")
set(LDFLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_MODULE_LINKER_FLAGS "")


cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME "kendryte-standalone-sdk")
##### include(macros) #####
macro(header_directories parent)
    file(GLOB_RECURSE newList ${parent}/*.h)
    set(dir_list "")
    foreach (file_path ${newList})
        get_filename_component(dir_path ${file_path} DIRECTORY)
        set(dir_list ${dir_list} ${dir_path})
    endforeach ()
    list(REMOVE_DUPLICATES dir_list)

    include_directories(${dir_list})
endmacro()

string(SUBSTRING ${CMAKE_BINARY_DIR} 0 1 CMAKE_BINARY_DIR_L)
string(TOUPPER ${CMAKE_BINARY_DIR_L} CMAKE_BINARY_DIR_L)
string(SUBSTRING ${CMAKE_BINARY_DIR} 1 -1 CMAKE_BINARY_DIR_R)
set(CMAKE_BINARY_DIRX "${CMAKE_BINARY_DIR_L}${CMAKE_BINARY_DIR_R}")

macro(add_source_files)
    file(GLOB_RECURSE newlist ${ARGN})

    foreach (filepath ${newlist})
        string(FIND ${filepath} ${CMAKE_BINARY_DIRX} found1)
        string(FIND ${filepath} "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries" found2)
        if ((NOT found1 EQUAL 0) AND (NOT found2 EQUAL 0))
            list(APPEND SOURCE_FILES ${filepath})
        endif ()
    endforeach ()
    set("SOURCE_FILES_${PROJECT_NAME}" "${SOURCE_FILES}" CACHE STRING "Dump files to compile for '${PROJECT_NAME}'. Not used by program.")
endmacro()

function(JOIN VALUES GLUE OUTPUT)
    string(REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
    string(REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
    set(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

function(removeDuplicateSubstring stringIn stringOut)
    separate_arguments(stringIn)
    list(REMOVE_DUPLICATES stringIn)
    string(REPLACE ";" " " stringIn "${stringIn}")
    set(${stringOut} "${stringIn}" PARENT_SCOPE)
endfunction()

macro(add_compile_flags WHERE)
    JOIN("${ARGN}" " " STRING_ARGS)
    if (${WHERE} STREQUAL C)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL CXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL LD)
        set(LDFLAGS "${LDFLAGS} ${STRING_ARGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${STRING_ARGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${STRING_ARGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL BOTH)
        add_compile_flags(C ${ARGN})
        add_compile_flags(CXX ${ARGN})

    elseif (${WHERE} STREQUAL ALL)
        add_compile_flags(C ${ARGN})
        add_compile_flags(CXX ${ARGN})
        add_compile_flags(LD ${ARGN})

    else ()
        message(FATAL_ERROR "add_compile_flags - only support: C, CXX, BOTH, LD, ALL")
    endif ()
endmacro()


##### include(ideSettings) #####
# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
else()
    if((NOT CMAKE_BUILD_TYPE STREQUAL "Debug") AND (NOT CMAKE_BUILD_TYPE STREQUAL "Release"))
        message(FATAL_ERROR "CMAKE_BUILD_TYPE must either be Debug or Release instead of ${CMAKE_BUILD_TYPE}")
    endif()
endif()
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type options are: Debug Release RelWithDebInfo MinSizeRel")

## - Debug & Release
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG=1)
endif()

## definitions in macros
add_definitions(-DCONFIG_LOG_LEVEL=LOG_INFO -DCONFIG_LOG_ENABLE -D__riscv64)
if(DEFINED KENDRYTE_IDE)
	set(CMAKE_COLOR_MAKEFILE FALSE)
else()
	add_definitions(-DCONFIG_LOG_COLORS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE CACHE INTERNAL "must set to true, never edit or IDEs will not work")


##### include(toolchain) #####
if (WIN32)
    set(BINARY_EXTENSION ".exe")
else ()
    set(BINARY_EXTENSION "")
endif ()

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_LINKER "ld")
set(CMAKE_AR "ar")
set(CMAKE_OBJCOPY "objcopy")
set(CMAKE_SIZE "size")
set(CMAKE_OBJDUMP "objdump")
set(CMAKE_RANLIB "ranlib")
set(CMAKE_NM "nm")
set(CMAKE_STRIP "strip")
set(CMAKE_C_COMPILER_AR "gcc-ar")
set(CMAKE_C_COMPILER_RANLIB "gcc-ranlib")
set(CMAKE_CXX_COMPILER_AR "gcc-ar")
set(CMAKE_CXX_COMPILER_RANLIB "gcc-ranlib")

set(CMAKE_TOOLCHAIN_PREFIX "riscv64-unknown-elf-" CACHE INTERNAL "this const is defined by toolchain")
set(CMAKE_EXECUTABLE_FORMAT "ELF" CACHE INTERNAL "this const is defined by toolchain")

foreach(CMD CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_LINKER CMAKE_AR CMAKE_OBJCOPY CMAKE_SIZE CMAKE_OBJDUMP CMAKE_RANLIB CMAKE_NM CMAKE_STRIP CMAKE_C_COMPILER_AR CMAKE_C_COMPILER_RANLIB CMAKE_CXX_COMPILER_AR CMAKE_CXX_COMPILER_RANLIB)
	set(${CMD} "${CMAKE_TOOLCHAIN_PREFIX}${${CMD}}${BINARY_EXTENSION}" CACHE FILEPATH "the path of ${CMD}" FORCE)
endforeach()

#execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTBEGIN_OBJ)
#execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTEND_OBJ)
#execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTI_OBJ)
#execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTN_OBJ)
#
#set(CMAKE_C_LINK_EXECUTABLE
#       "<CMAKE_C_COMPILER>  <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> \"${CRTI_OBJ}\" \"${CRTBEGIN_OBJ}\" <OBJECTS> \"${CRTEND_OBJ}\" \"${CRTN_OBJ}\" -o <TARGET> <LINK_LIBRARIES>"
#       CACHE STRING "CMAKE_C_LINK_EXECUTABLE"
#       )
#
#set(CMAKE_CXX_LINK_EXECUTABLE
#       "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> \"${CRTI_OBJ}\" \"${CRTBEGIN_OBJ}\" <OBJECTS> \"${CRTEND_OBJ}\" \"${CRTN_OBJ}\" -o <TARGET> <LINK_LIBRARIES>"
#       CACHE STRING "CMAKE_CXX_LINK_EXECUTABLE"
#       )
#



##### flags from config json #####
add_compile_flags(C
  "-std=gnu11"
  "-Wno-pointer-to-int-cast"
  "-Wno-old-style-declaration"
)
add_compile_flags(CXX
  "-std=gnu++17"
)
add_compile_flags(BOTH
  "-mcmodel=medany"
  "-fno-common"
  "-ffunction-sections"
  "-fdata-sections"
  "-fstrict-volatile-bitfields"
  "-fno-zero-initialized-in-bss"
  "-Os"
  "-ggdb"
  "-Wall"
  "-Werror=all"
  "-Wno-error=unused-function"
  "-Wno-error=unused-but-set-variable"
  "-Wno-error=unused-variable"
  "-Wno-error=deprecated-declarations"
  "-Wextra"
  "-Werror=frame-larger-than=65536"
  "-Wno-unused-parameter"
  "-Wno-sign-compare"
  "-Wno-error=missing-braces"
  "-Wno-error=return-type"
  "-Wno-error=pointer-sign"
  "-Wno-missing-braces"
  "-Wno-strict-aliasing"
  "-Wno-implicit-fallthrough"
  "-Wno-missing-field-initializers"
)
add_compile_flags(LD
  "-static"
  "-Wl,-static"
  "-Wl,-EL"
)
##### internal flags #####
##### include(coreFlags) #####
add_compile_flags(LD -nostartfiles -Wl,--gc-sections)
add_compile_flags(BOTH -mcmodel=medany)


add_compile_flags(LD
  -T "${CMAKE_CURRENT_LIST_DIR}/lds/kendryte.ld"
)
##### include(fix9985) #####
### http://www.cmake.org/Bug/view.php?id=9985
string(REPLACE "-rdynamic" "" _C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")
string(REPLACE "-rdynamic" "" _CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${_C_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${_CXX_FLAGS}")


##### Main Section #####
message("======== PROJECT ========")
project(${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE TRUE)
## add include from self and dependency
include_directories(
  "${CMAKE_CURRENT_LIST_DIR}/lib/drivers/include"
  "${CMAKE_CURRENT_LIST_DIR}/lib/bsp/include"
  "${CMAKE_CURRENT_LIST_DIR}/lib/utils/include"
  "${CMAKE_CURRENT_LIST_DIR}/lib/freertos/include"
  "${CMAKE_CURRENT_LIST_DIR}/lib/freertos/conf"
  "${CMAKE_CURRENT_LIST_DIR}/lib/freertos/portable"
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_speex/include"
)
## there is no headers
## add source from config json
add_source_files(
  "${CMAKE_CURRENT_LIST_DIR}/config/fpioa-config.c"
  "${CMAKE_CURRENT_LIST_DIR}/config/fpioa-config.h"
)
add_source_files(
  "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.h"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.s"
  "${CMAKE_CURRENT_LIST_DIR}/lib/*.S"
)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_speex" "kendryte_speex")
##### include(asm.cmake) #####
FILE(GLOB_RECURSE ASSEMBLY_FILES
        "${CMAKE_CURRENT_LIST_DIR}/*.s"
        "${CMAKE_CURRENT_LIST_DIR}/*.S"
)

SET_PROPERTY(SOURCE ${ASSEMBLY_FILES} PROPERTY LANGUAGE C)
SET_SOURCE_FILES_PROPERTIES(${ASSEMBLY_FILES} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D __riscv64")


##### include(asm.cmake) END #####
## final create kendryte-standalone-sdk executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_property(GLOBAL PROPERTY JOB_POOLS single_debug=1)
set_property(TARGET ${PROJECT_NAME} PROPERTY JOB_POOL_COMPILE single_debug)
set_property(TARGET ${PROJECT_NAME} PROPERTY JOB_POOL_LINK single_debug)
## set properties
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE "C")
## no definitions
target_link_libraries(${PROJECT_NAME} PUBLIC -Wl,--start-group gcc m c -Wl,--end-group)
## dependencies link
target_link_libraries(${PROJECT_NAME} PUBLIC
  -Wl,--start-group
    "kendryte_speex"
  -Wl,--end-group
)
##### include(afterProject) #####
removeDuplicateSubstring("${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)
removeDuplicateSubstring("${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)


##### include(fix9985) #####
### http://www.cmake.org/Bug/view.php?id=9985
string(REPLACE "-rdynamic" "" _C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")
string(REPLACE "-rdynamic" "" _CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${_C_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${_CXX_FLAGS}")


##### include(flash) #####
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${SUFFIX} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating .bin file ...")


##### include(dumpConfig) #####
set(CMAKE_SYSTEM_NAME "Generic")

message("")
message("Project: ${PROJECT_NAME}")
message("  LIST_FILE=${CMAKE_PARENT_LIST_FILE}")
message("  TOOLCHAIN=${TOOLCHAIN}")
message("  KENDRYTE_IDE=${KENDRYTE_IDE}")
message("  BUILDING_SDK=${BUILDING_SDK}")
message("  _CMAKE_TOOLCHAIN_PREFIX=${_CMAKE_TOOLCHAIN_PREFIX}")
message("  _CMAKE_TOOLCHAIN_LOCATION=${_CMAKE_TOOLCHAIN_LOCATION}")


message("")
message("  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("  CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("  CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("  CMAKE_LINKER=${CMAKE_LINKER}")
message("  CMAKE_OBJCOPY=${CMAKE_OBJCOPY}")
message("  CMAKE_OBJDUMP=${CMAKE_OBJDUMP}")
message("  CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")

message("")
message("  CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message("  CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
message("  LDFLAGS=${LDFLAGS}")
message("  CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

message("Makefile created.")
message("")
message("")


message("")
message("  ${PROJECT_NAME} :: SOURCE_FILES=${SOURCE_FILES}")
