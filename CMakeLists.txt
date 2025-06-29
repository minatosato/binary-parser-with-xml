cmake_minimum_required(VERSION 3.20)
project(binary_parser_with_xml)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(tinyxml2 CONFIG)
if(NOT tinyxml2_FOUND)
    # Try to find TinyXML2 manually
    find_path(TINYXML2_INCLUDE_DIR tinyxml2.h PATHS /opt/homebrew/include /usr/local/include)
    find_library(TINYXML2_LIBRARY tinyxml2 PATHS /opt/homebrew/lib /usr/local/lib)
    if(TINYXML2_INCLUDE_DIR AND TINYXML2_LIBRARY)
        message(STATUS "Found TinyXML2: ${TINYXML2_LIBRARY}")
    else()
        message(FATAL_ERROR "TinyXML2 not found")
    endif()
endif()
find_package(GTest CONFIG)

# Binary parser library
add_library(binary_parser
    src/binary_parser/binary_parser.cpp
    src/binary_parser/xml_struct_parser.cpp
)

target_include_directories(binary_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/binary_parser
)

if(tinyxml2_FOUND)
    target_link_libraries(binary_parser PUBLIC tinyxml2::tinyxml2)
else()
    target_include_directories(binary_parser PUBLIC ${TINYXML2_INCLUDE_DIR})
    target_link_libraries(binary_parser PUBLIC ${TINYXML2_LIBRARY})
endif()

# Binary parser executable
add_executable(parse_binary
    src/binary_parser/main.cpp
)

target_link_libraries(parse_binary PRIVATE
    binary_parser
)

# Tests
if(GTest_FOUND)
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
endif()