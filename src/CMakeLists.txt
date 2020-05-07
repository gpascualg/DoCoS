find_package (Threads REQUIRED)

add_library(docos_core STATIC 
    gx/camera/camera.cpp
    gx/mesh/mesh.cpp
    gx/shader/program.cpp
    io/memmap.cpp
    docos.cpp)

add_executable(docos main.cpp)

target_link_libraries(docos_core PUBLIC Threads::Threads)
target_compile_features(docos_core PUBLIC cxx_std_20)
target_include_directories(docos_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(docos_core PUBLIC 
    BOOST_ERROR_CODE_HEADER_ONLY
    BOOST_SYSTEM_NO_DEPRECATED
    BOOST_ALL_NO_LIB
    BOOST_ASIO_NO_DEPRECATED)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(docos_core PUBLIC -fcoroutines-ts -stdlib=libc++)
    target_link_options(docos_core INTERFACE -stdlib=libc++ -lc++abi -lstdc++)
endif()

target_link_libraries(docos PUBLIC docos_core)
