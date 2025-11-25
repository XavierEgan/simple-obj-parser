cmake_minimum_required(VERSION 3.18)

project(
    ObjectParser
    VERSION 3.0.0
    LANGUAGES CXX CUDA
)

# get sources
file(
	GLOB_RECURSE OBJ_PARSER_SOURCES CMAKE_CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
# find include file
set(OBJ_PARSER_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# make the library
add_library(OBJ_PARSER ${OBJ_PARSER_SOURCES})

# link includes
target_include_directories(OBJ_PARSER PUBLIC "${OBJ_PARSER_INCLUDE}")

# make alias for OBJECT_PARSER
add_library(OBJECT_PARSER::obj_parser ALIAS OBJ_PARSER)

include(CTest)
enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

add_executable(ObjParserTests ${TEST_SOURCES})
target_link_libraries(
    ObjParserTests 
    PUBLIC
        OBJECT_PARSER::obj_parser
        gtest_main
)

target_include_directories(
    ObjParserTests
    PUBLIC
        "${CMAKE_SOURCE_DIR}/tests"
)