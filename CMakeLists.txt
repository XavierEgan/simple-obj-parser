cmake_minimum_required(VERSION 3.18)

project(
    ObjectParser
    VERSION 3.0.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)

include(CTest)
enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)


add_executable(ObjParserTests
    tests/test_main.cpp
)

target_link_libraries(ObjParserTests
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(ObjParserTests)