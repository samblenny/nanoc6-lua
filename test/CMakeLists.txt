# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

# Include the components directory of the main application:
set(EXTRA_COMPONENT_DIRS "../components")

# Set the components to include the tests for.
set(TEST_COMPONENTS "utf8" CACHE STRING "List of components to test")

# Force gcc to rewrite __FILE__ with shorter relative paths so the Unity
# test runner output doesn't use a super-long file prefix on every test.
# Since CMAKE_SOURCE_DIR is .../test rather than the project root directory,
# we need to remove the /test part to get a match on the long prefix.
#
string(REGEX REPLACE "/test$" "" PARENT_DIR "${CMAKE_SOURCE_DIR}")
add_compile_options(-ffile-prefix-map=${PARENT_DIR}=.)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(unit_tests)
