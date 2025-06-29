cmake_minimum_required(VERSION 3.20)

# Find the binary_parser library from the parent project
find_library(BINARY_PARSER_LIB binary_parser PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../build NO_DEFAULT_PATH)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../src/binary_parser)

# Extreme test executable
add_executable(test_extreme_parser
    test_extreme_parser.cpp
)

# Link with binary_parser library
target_link_libraries(test_extreme_parser PRIVATE
    ${BINARY_PARSER_LIB}
)

# Set C++ standard
set_target_properties(test_extreme_parser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)