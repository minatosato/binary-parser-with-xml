cmake_minimum_required(VERSION 3.20)
project(binary_parser_with_xml)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(pugixml CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Binary parser library
add_library(binary_parser
    src/binary_parser/binary_parser.cpp
    src/binary_parser/xml_struct_parser.cpp
)

target_include_directories(binary_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/binary_parser
)

target_link_libraries(binary_parser PUBLIC
    pugixml::pugixml
)

# Binary parser executable
add_executable(parse_binary
    src/binary_parser/main.cpp
)

target_link_libraries(parse_binary PRIVATE
    binary_parser
)

# Tests
enable_testing()

add_executable(test_binary_parser
    tests/test_binary_parser.cpp
)

target_link_libraries(test_binary_parser PRIVATE
    binary_parser
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME test_binary_parser COMMAND test_binary_parser)