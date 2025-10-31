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

# Fetch and build ImageMagick 6
message(STATUS "Fetching and building ImageMagick 6.9.13-0 (Magick++)")

include(ExternalProject)

set(IMAGEMAGICK6_INSTALL_DIR ${CMAKE_BINARY_DIR}/imagemagick6-install)

ExternalProject_Add(ImageMagick6
  GIT_REPOSITORY https://github.com/ImageMagick/ImageMagick6.git
  GIT_TAG 6.9.13-0
  PREFIX ${CMAKE_BINARY_DIR}/ImageMagick6
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ./configure
      CC=${CMAKE_C_COMPILER}
      CXX=${CMAKE_CXX_COMPILER}
      --disable-dependency-tracking
      --disable-docs
      --without-perl
      --enable-shared
      --with-magick-plus-plus
      --prefix=${IMAGEMAGICK6_INSTALL_DIR}
  BUILD_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} -j${NPROC}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} install
  UPDATE_DISCONNECTED TRUE
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
)


# Where it will be installed
set(IM6_INCLUDE_DIR ${IMAGEMAGICK6_INSTALL_DIR}/include/ImageMagick-6)
set(IM6_LIB_DIR ${IMAGEMAGICK6_INSTALL_DIR}/lib)

# Create an interface target for linking
add_library(imagemagick6 INTERFACE)
add_dependencies(imagemagick6 ImageMagick6)
target_include_directories(imagemagick6 INTERFACE ${IM6_INCLUDE_DIR})
target_link_directories(imagemagick6 INTERFACE ${IM6_LIB_DIR})
target_link_libraries(imagemagick6 INTERFACE Magick++-6.Q16 MagickCore-6.Q16 MagickWand-6.Q16)

# Gather all code source files automatically
file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# My executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
add_dependencies(${PROJECT_NAME} ImageMagick6)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${IM6_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE imagemagick6)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  MAGICKCORE_QUANTUM_DEPTH=16
  MAGICKCORE_HDRI_ENABLE=0
)

# Print summary
message(STATUS "ImageMagick include dir: ${IM6_INCLUDE_DIR}")
message(STATUS "ImageMagick lib dir: ${IM6_LIB_DIR}")
