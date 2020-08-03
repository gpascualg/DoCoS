cmake_minimum_required(VERSION 3.11...3.16)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(DoCoS VERSION 0.1 LANGUAGES CXX)

option(BUILD_SAMPLES        "Build samples"             ON)
option(BUILD_SAMPLE_CLIENT  "Build client sample"       OFF)
option(BUILD_SAMPLE_SERVER  "Build server sample"       ON)
option(Boost_USE_STATIC_LIBS "Use Boost static libs"    ON)
set(BOOST_VERSION "1.73" CACHE STRING "Boost version")

add_subdirectory(src)
add_subdirectory(dep)

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
