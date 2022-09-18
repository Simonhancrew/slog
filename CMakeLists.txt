cmake_minimum_required(VERSION 3.12)

project(Slog VERSION 1.0.0 LANGUAGES C CXX)

# Specified the language standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


##--------------------- Compile Options ------------------------------------##
add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++17")

# Set build type
# set(CMAKE_BUILD_TYPE Debug)  # Use `cmake -DCMAKE_BUILD_TYPE=Debug` more better
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Compile options for Debug variant
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Compile options for Release variant
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

message(STATUS "Compile options for c: ${CMAKE_C_FLAGS}")
message(STATUS "Compile options for c++: ${CMAKE_CXX_FLAGS}")

##--------------------- Global Macros --------------------------------------##
# add_definitions(-DDEBUG -DREAL_COOL_ENGINEER)

##--------------------- Include directories --------------------------------##
# include_directories(src/c)

##--------------------- Build target ---------------------------------------##
