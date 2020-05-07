# Copyright 2018 Peter Dimov
# Copyright 2018 Andrey Semashev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

# Partial (add_subdirectory only) and experimental CMake support
# Subject to change; please do not rely on the contents of this file yet.

cmake_minimum_required(VERSION 3.5)

project(Boost@CURRENT_BOOST_SUBMODULE@ LANGUAGES CXX)

add_library(boost_@CURRENT_BOOST_SUBMODULE@ INTERFACE)
add_library(Boost::@CURRENT_BOOST_SUBMODULE@ ALIAS boost_@CURRENT_BOOST_SUBMODULE@)

target_include_directories(boost_@CURRENT_BOOST_SUBMODULE@ INTERFACE include)

# target_link_libraries(boost_@CURRENT_BOOST_SUBMODULE@
#     INTERFACE
#         Boost::assert
#         Boost::concept_check
#         Boost::config
#         Boost::conversion
#         Boost::core
#         Boost::detail
#         Boost::function_types
#         Boost::fusionz
#         Boost::mpl
#         Boost::optional
#         Boost::smart_ptr
#         Boost::static_assert
#         Boost::type_traits
#         Boost::utility
# )
