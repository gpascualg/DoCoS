include(FetchContent)


find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS fiber)
target_include_directories(docos_core PUBLIC ${Boost_INCLUDE_DIRS})
target_link_directories(docos_core PUBLIC ${Boost_LIBRARY_DIRS})
target_link_libraries(docos_core PUBLIC Boost::fiber)

