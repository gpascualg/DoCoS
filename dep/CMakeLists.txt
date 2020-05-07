include(FetchContent)


set(BOOST_DEPENDENCIES
    # Lockfree dependencies
    # lockfree core config assert static_assert type_traits mpl preprocessor
    # array throw_exception functional align parameter atomic detail
    # utility predef winapi tti fusion mp11

    # Asio dependencies
    asio system date_time smart_ptr bind regex numeric_conversion

    # Pool dependencies
    pool integer type_traits

    # hash_combine
    container_hash

    # Cicular buffer
    circular_buffer move concept_check

    # range
    range iterator core config assert static_assert predef optional tuple mpl 
    preprocessor detail utility exception throw_exception
)

foreach (dep ${BOOST_DEPENDENCIES})
    # Most package don't yet have a CMakeLists
    FetchContent_Declare(
        Boost${dep}
        GIT_REPOSITORY https://github.com/boostorg/${dep}
        GIT_TAG        boost-${BOOST_VERSION}.0
    )

    FetchContent_GetProperties(boost${dep})
    if(NOT ${dep}_POPULATED)
        FetchContent_Populate(boost${dep})
        
        set(CURRENT_BOOST_SUBMODULE ${dep})
        configure_file (
            "${CMAKE_CURRENT_SOURCE_DIR}/boost.cmake"
            ${boost${dep}_SOURCE_DIR}/CMakeLists.txt
        )
        unset(CURRENT_BOOST_SUBMODULE)

        add_subdirectory(${boost${dep}_SOURCE_DIR} ${boost${dep}_BINARY_DIR})
    endif()

    target_link_libraries(docos_core PUBLIC Boost::${dep})
endforeach()

FetchContent_Declare(
    cppcoro
    GIT_REPOSITORY https://github.com/lewissbaker/cppcoro
    GIT_TAG        master
)

FetchContent_GetProperties(cppcoro)
if(NOT ${dep}_POPULATED)
    FetchContent_Populate(cppcoro)
    
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/cppcoro.cmake"
        ${cppcoro_SOURCE_DIR}/CMakeLists.txt
        COPYONLY
    )
    
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/cppcoro.config.cmake.in"
        ${cppcoro_SOURCE_DIR}/config.cmake.in
        COPYONLY
    )

    add_subdirectory(${cppcoro_SOURCE_DIR} ${cppcoro_BINARY_DIR})
endif()

target_link_libraries(docos_core PUBLIC cppcoro)



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




