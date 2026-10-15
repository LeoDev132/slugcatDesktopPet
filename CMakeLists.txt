cmake_minimum_required(VERSION 3.10.0)

set(ProjectName "slug_pet")

project(ProjectName)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SFML_STATIC_LIBRARIES TRUE)


set(RAYLIB_DIR "$ENV{HOME}/lib/cmake/raylib")
find_package(RAYLIB 6.0)
add_executable(${ProjectName} main.cpp Slugcat.hpp)

target_include_directories(${ProjectName} PRIVATE $ENV{HOME}/include)
target_link_libraries(${ProjectName}  raylib)
