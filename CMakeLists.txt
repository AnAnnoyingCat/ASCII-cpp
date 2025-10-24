cmake_minimum_required(VERSION 3.6)
cmake_policy(VERSION 3.6...4.0)

project(asciirenderer)

# ImageMagick
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
include_directories(${ImageMagick_INCLUDE_DIRS})
add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
add_definitions(-DMAGICKCORE_HDRI_ENABLE=0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(VORPALINE_PLATFORM Linux64-gcc-dynamic)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/include)

# project files
file(GLOB H_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB SRCFILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# build executable
add_executable(${PROJECT_NAME} main.cpp  ${H_FILES} ${SRCFILES})

target_link_libraries(${PROJECT_NAME} ${ImageMagick_LIBRARIES})
