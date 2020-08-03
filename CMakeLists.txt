cmake_minimum_required(VERSION 3.11...3.16)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(DoCoS VERSION 0.1 LANGUAGES CXX)

set(Boost_USE_STATIC_LIBS ON CACHE STRING "Boost static libs")
set(BOOST_VERSION "1.73" CACHE STRING "Boost version")

add_subdirectory(src)
add_subdirectory(dep)
add_subdirectory(samples)
