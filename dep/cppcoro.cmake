cmake_minimum_required(VERSION 3.5)


project(cppcoro)

add_library(cppcoro lib/ipv4_address.cpp
    lib/cancellation_registration.cpp
    lib/cancellation_source.cpp
    lib/ipv4_endpoint.cpp
    lib/spin_mutex.cpp
    lib/ipv6_address.cpp
    lib/auto_reset_event.cpp
    lib/ip_endpoint.cpp
    lib/async_mutex.cpp
    lib/ipv6_endpoint.cpp
    lib/static_thread_pool.cpp
    lib/cancellation_token.cpp
    lib/async_auto_reset_event.cpp
    lib/async_manual_reset_event.cpp
    lib/lightweight_manual_reset_event.cpp
    lib/spin_wait.cpp
    lib/ip_address.cpp
    lib/cancellation_state.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(cppcoro PUBLIC -fcoroutines-ts -stdlib=libc++)
    target_link_options(cppcoro INTERFACE -stdlib=libc++ -lc++abi -lstdc++)
endif()

target_include_directories(cppcoro
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(cppcoro PRIVATE cxx_std_20)

string(TOLOWER cppcoro TARGET_LOWER)

include(CMakePackageConfigHelpers)
set(config_install_dir lib/cmake/${TARGET_LOWER})
set(version_config ${PROJECT_BINARY_DIR}/${TARGET_LOWER}-config-version.cmake)
set(project_config ${PROJECT_BINARY_DIR}/${TARGET_LOWER}-config.cmake)
set(targets_export_name ${TARGET_LOWER}-targets)

write_basic_package_version_file(
    ${version_config}
    VERSION 0.0.1
    COMPATIBILITY AnyNewerVersion)
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/config.cmake.in
    ${project_config}
    INSTALL_DESTINATION ${config_install_dir})
export(TARGETS cppcoro FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

install(TARGETS cppcoro
    EXPORT ${targets_export_name}
    RUNTIME DESTINATION bin/
    LIBRARY DESTINATION lib/
    ARCHIVE DESTINATION lib/)

install(FILES ${project_config} ${version_config} DESTINATION ${config_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${config_install_dir})

if (EXISTS ${PROJECT_SOURCE_DIR}/include)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION .)
endif()