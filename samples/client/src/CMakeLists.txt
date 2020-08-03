add_executable(docos_sample_client 
    gx/camera/camera.hpp
    gx/camera/camera.cpp
    gx/mesh/mesh.hpp
    gx/mesh/mesh.cpp
    gx/shader/program.hpp
    gx/shader/program.cpp
    main.cpp)

target_include_directories(docos_sample_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(docos_sample_client PRIVATE docos_core)

add_custom_command(
    TARGET docos_sample_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/../resources
            ${CMAKE_CURRENT_BINARY_DIR}/resources)
