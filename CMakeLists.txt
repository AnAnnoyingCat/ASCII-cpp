cmake_minimum_required(VERSION 3.20)

project(asciirenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use all available cores
include(ProcessorCount)
ProcessorCount(NPROC)
if(NOT NPROC OR NPROC EQUAL 0)
  set(NPROC 1)
endif()

# Fetch and build ImageMagick
message(STATUS "Fetching and building ImageMagick 6.9.12-98 (Magick++)")

include(ExternalProject)
ExternalProject_Add(
  ImageMagick6
  URL https://download.imagemagick.org/archive/releases/ImageMagick-6.9.12-98.tar.xz
  PREFIX ${CMAKE_BINARY_DIR}/ImageMagick6
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./configure
      --disable-dependency-tracking
      --disable-docs
      --without-perl
      --enable-shared
      --with-magick-plus-plus
      --prefix=<INSTALL_DIR>
  BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} -j${NPROC}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} install
  UPDATE_DISCONNECTED TRUE
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
)

# Where it will be installed
ExternalProject_Get_Property(ImageMagick6 install_dir)

# Add Magick++ include/lib dirs after install
set(IM6_INCLUDE_DIR ${install_dir}/include/ImageMagick-6)
set(IM6_LIB_DIR ${install_dir}/lib)

# My executable
add_executable(${PROJECT_NAME} main.cpp)

add_dependencies(${PROJECT_NAME} ImageMagick6)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${IM6_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/external/include
)

find_library(IM6_LIB
  NAMES Magick++-6.Q16HDRI Magick++-6.Q16 Magick++
  PATHS ${IM6_LIB_DIR}
  REQUIRED
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${IM6_LIB})

# Print summary

message(STATUS "ImageMagick include dir: ${IM6_INCLUDE_DIR}")
message(STATUS "ImageMagick lib: ${IM6_LIB}")
