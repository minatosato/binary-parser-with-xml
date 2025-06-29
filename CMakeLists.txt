cmake_minimum_required(VERSION 3.10)
project(binary-parser-with-xml)

# C++17を有効化
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# OSに応じた依存関係の設定
if(WIN32)
    # Windowsの場合はvcpkgを使用
    if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    endif()
    
    # パッケージを探す
    find_package(GTest CONFIG REQUIRED)
    find_package(tinyxml2 CONFIG REQUIRED)
else()
    # macOSの場合はシステムのパッケージを使用
    find_package(GTest REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(TINYXML2 REQUIRED tinyxml2)
    
    if(APPLE)
        # tinyxml2のパスを明示的に設定
        link_directories(/opt/homebrew/Cellar/tinyxml2/11.0.0/lib)
        include_directories(/opt/homebrew/Cellar/tinyxml2/11.0.0/include)
    endif()
endif()

# ソースファイル
set(SOURCES
    src/binary_parser/binary_parser.cpp
    src/binary_parser/xml_struct_parser.cpp
    src/binary_parser/json_converter.cpp
    src/binary_parser/main.cpp
    src/json/json_value.cpp
)

# ヘッダーファイルのディレクトリ
include_directories(${PROJECT_SOURCE_DIR}/src)

# 実行ファイルの作成
add_executable(parse_binary ${SOURCES})
target_include_directories(parse_binary PRIVATE ${TINYXML2_INCLUDE_DIRS})
if(WIN32)
    target_link_libraries(parse_binary PRIVATE tinyxml2::tinyxml2)
else()
    target_link_libraries(parse_binary PRIVATE tinyxml2)
endif()

# テストの設定
enable_testing()

# テストソースファイル
set(TEST_SOURCES
    tests/unit/json/test_json_value.cpp
    tests/unit/test_json_converter.cpp
    tests/unit/test_binary_parser.cpp
    tests/unit/test_binary_parser_endianness.cpp
    tests/unit/test_binary_parser_logic.cpp
    tests/unit/test_char_type.cpp
    tests/unit/test_signed_types.cpp
    src/json/json_value.cpp
    src/binary_parser/binary_parser.cpp
    src/binary_parser/xml_struct_parser.cpp
    src/binary_parser/json_converter.cpp
)

# テスト実行ファイルの作成
add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
)
if(WIN32)
    target_link_libraries(run_tests PRIVATE 
        GTest::gtest 
        GTest::gtest_main
        tinyxml2::tinyxml2
    )
else()
    target_link_libraries(run_tests PRIVATE 
        GTest::GTest 
        GTest::Main
        tinyxml2
    )
endif()

# テストの登録
add_test(NAME unit_tests COMMAND run_tests)
