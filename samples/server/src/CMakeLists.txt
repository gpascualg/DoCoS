add_executable(docos_sample_server 
    core/server.hpp
    core/server.cpp
    maps/map.hpp
    maps/map.cpp
    main.cpp)

target_include_directories(docos_sample_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(docos_sample_server PRIVATE docos_core)

add_custom_command(
    TARGET docos_sample_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/../resources
            ${CMAKE_CURRENT_BINARY_DIR}/resources)
