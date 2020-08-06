include(FetchContent)


FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        0.9.9.8
)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

target_link_libraries(docos_sample_server PUBLIC glm)



FetchContent_Declare(
    kumo
    GIT_REPOSITORY https://github.com/AErbis/Kumo.git
    GIT_TAG        master
)

FetchContent_GetProperties(kumo)
if(NOT kumo_POPULATED)
    FetchContent_Populate(kumo)
endif()

