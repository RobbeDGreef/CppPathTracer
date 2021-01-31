cmake_minimum_required(VERSION 3.1)
project(raytracer)

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(Threads)
add_executable(raytracer ${SOURCES})
target_link_libraries(raytracer ${CMAKE_THREAD_LIBS_INIT})