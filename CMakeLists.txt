cmake_minimum_required(VERSION 3.21.2)
project(pn2hda VERSION 0.1.0 LANGUAGES C)
set(active_log YES CACHE BOOL "activate logs")
add_executable(pn2hda "")
set_target_properties(pn2hda
  PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON)
find_package(LibXml2 REQUIRED)
target_link_libraries(pn2hda
  PRIVATE
    ${LIBXML2_LIBRARIES})
target_include_directories(pn2hda
  PRIVATE
    "include/"
    ${LIBXML2_INCLUDE_DIRS})
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
if(NOT active_log)
  add_definitions("-DNOLOG")
endif()
target_compile_options(pn2hda
  PRIVATE
    -Wall -Wextra -Werror -pedantic --std=c99 -Wvla)
file(GLOB sources "src/*/*.c")
target_sources(pn2hda
  PRIVATE
    "src/main.c"
    ${sources})
