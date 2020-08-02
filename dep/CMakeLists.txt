include(FetchContent)


find_package(Boost 1.73 REQUIRED COMPONENTS fiber)
target_include_directories(docos_core PUBLIC ${Boost_INCLUDE_DIRS})
target_link_directories(docos_core PUBLIC ${Boost_LIBRARY_DIRS})
target_link_libraries(docos_core PUBLIC Boost::fiber)


FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.2
)

FetchContent_GetProperties(glfw)
if(NOT ${dep}_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

target_link_libraries(docos_core PUBLIC glfw)




FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad
    GIT_TAG        master
)

FetchContent_GetProperties(glad)
if(NOT ${dep}_POPULATED)
    FetchContent_Populate(glad)
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR})
endif()

target_link_libraries(docos_core PUBLIC glad)



FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        0.9.9.8
)

FetchContent_GetProperties(glm)
if(NOT ${dep}_POPULATED)
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

target_link_libraries(docos_core PUBLIC glm)




