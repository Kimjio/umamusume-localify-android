cmake_minimum_required(VERSION 3.18.1)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

add_definitions(-DRIRU_MODULE)

configure_file(template/config.cpp config.cpp)

macro(SET_OPTION option value)
    set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()

SET_OPTION(DOBBY_DEBUG OFF)
SET_OPTION(DOBBY_GENERATE_SHARED OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Dobby)

include_directories(${CMAKE_CURRENT_LIST_DIR}/rapidjson/include)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SQLiteCpp)

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 23)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fexceptions -fno-rtti")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

if (${API} STREQUAL "zygisk")
    include_directories(include)

    add_library(${MODULE_NAME} SHARED main_zygisk.cpp hook.cpp il2cpp/il2cpp_symbols.cpp il2cpp_hook.cpp localify/localify.cpp logger/logger.cpp ${CMAKE_CURRENT_BINARY_DIR}/config.cpp)
    target_link_libraries(${MODULE_NAME} log dobby SQLiteCpp sqlite3)
elseif (${API} STREQUAL "riru")
    find_package(riru REQUIRED CONFIG)

    include_directories(include)

    add_library(${MODULE_NAME} SHARED main_riru.cpp hook.cpp il2cpp/il2cpp_symbols.cpp il2cpp_hook.cpp localify/localify.cpp logger/logger.cpp ${CMAKE_CURRENT_BINARY_DIR}/config.cpp)
    target_link_libraries(${MODULE_NAME} log dobby SQLiteCpp sqlite3 riru::riru)
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${MODULE_NAME}.so")
endif ()
