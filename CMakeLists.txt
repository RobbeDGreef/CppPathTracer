cmake_minimum_required(VERSION 3.2)
project(raytracer)

# Add argparse library
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fno-omit-frame-pointer -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -Wall -Werror -fno-omit-frame-pointer -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")

# Enable debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Enable optimizations
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Enable address sanitizer
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(Threads)
add_executable(raytracer ${SOURCES})
target_link_libraries(raytracer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(raytracer argparse)
