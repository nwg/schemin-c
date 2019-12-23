cmake_minimum_required(VERSION 3.10)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

include (utils.cmake)
disallow_intree_builds()

# set the project name and version
project(Schemin VERSION 0.1)

add_executable(schemin src/schemin.c)
set_property(TARGET schemin PROPERTY C_STANDARD 11)

configure_file(version.h.in version.h)

target_include_directories(
  schemin PUBLIC
  "${PROJECT_BINARY_DIR}"
)

ExternalProject_Add(utf8proc
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/utf8proc"
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third-party/utf8proc
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

ExternalProject_Get_Property(utf8proc install_dir)
include_directories(${install_dir}/include)
add_dependencies(schemin utf8proc)
target_link_libraries(schemin ${install_dir}/lib/libutf8proc.a)
