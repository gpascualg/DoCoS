find_package (Threads REQUIRED)

add_library(docos_core STATIC 
    common/result_of.hpp
    common/types.hpp
    containers/concepts.hpp
    containers/dictionary.hpp
    containers/plain_pool.hpp
    containers/pool_item.hpp
    containers/pooled_static_vector.hpp
    containers/thread_local_tasks.cpp
    containers/thread_local_tasks.hpp
    containers/ticket.hpp
    entity/entity.hpp
    entity/scheme.hpp
    entity/transform.hpp
    fiber/exclusive_work_stealing.hpp
    fiber/exclusive_work_stealing_impl.hpp
    fiber/exclusive_shared_work.hpp
    fiber/exclusive_shared_work_impl.hpp
    fiber/yield.hpp
    fiber/detail/yield.hpp
    ids/generator.hpp
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

target_link_libraries(docos_core PUBLIC Threads::Threads)
target_compile_features(docos_core PUBLIC cxx_std_20)
target_include_directories(docos_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(docos_core PUBLIC -stdlib=libc++)
    target_link_options(docos_core INTERFACE -stdlib=libc++ -lc++abi -lc++)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(docos_core PUBLIC _UNICODE UNICODE)
    target_compile_options(docos_core PUBLIC /MP)
endif()
