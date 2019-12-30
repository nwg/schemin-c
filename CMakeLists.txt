cmake_minimum_required(VERSION 3.10)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

include (utils.cmake)
disallow_intree_builds()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the project name and version
project(Schemin LANGUAGES C VERSION 0.1)

add_executable(schemin src/schemin.c src/parser.c src/prettyprint.c src/system.c src/memory.c)
set_property(TARGET schemin PROPERTY C_STANDARD 11)

configure_file(version.h.in version.h)

target_include_directories(
  schemin PUBLIC
  "${PROJECT_BINARY_DIR}"
)

set(third_party_destdir ${PROJECT_BINARY_DIR}/third-party-prefix)
ExternalProject_Add(utf8proc
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/utf8proc"
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third-party/utf8proc
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${third_party_destdir}
)

include_directories(${third_party_destdir}/include)
add_dependencies(schemin utf8proc)
target_link_libraries(schemin ${third_party_destdir}/lib/libutf8proc.a)
