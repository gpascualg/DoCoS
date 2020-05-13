find_package (Threads REQUIRED)

add_library(docos_core STATIC 
    common/types.hpp
    containers/concepts.hpp
    containers/dictionary.hpp
    containers/pool_item.hpp
    containers/pooled_static_vector.hpp
    containers/ticket.hpp
    entity/entity.hpp
    entity/scheme.hpp
    entity/transform.hpp
    gx/camera/camera.hpp
    gx/camera/camera.cpp
    gx/mesh/mesh.hpp
    gx/mesh/mesh.cpp
    gx/shader/program.hpp
    gx/shader/program.cpp
    io/memmap.hpp
    io/memmap.cpp
    traits/base_dic.hpp
    traits/contains.hpp
    traits/has_type.hpp
    traits/shared_function.hpp
    traits/without_duplicates.hpp
    updater/executor.hpp
    updater/updater.hpp
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
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(docos_core PUBLIC _UNICODE UNICODE)
    target_link_libraries(docos PUBLIC synchronization.lib)
endif()

target_link_libraries(docos PUBLIC docos_core)

add_custom_command(
    TARGET docos POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_BINARY_DIR}/resources)
