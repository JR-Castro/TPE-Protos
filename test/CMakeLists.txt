cmake_minimum_required(VERSION 3.0)
project(TPE-Protos C)

message("=== test/CMake")

include(CTest)

set(CTEST_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CTEST_SOURCE_DIRECTORY: ${CTEST_SOURCE_DIRECTORY}")

# Shared variables
load_cache(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeCache.txt)

# Add the include directories
include_directories(../src/include)

## PARSER TEST
add_executable(
        parser_test
        ${CTEST_SOURCE_DIRECTORY}/parser_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(parser_test check m)
add_test(NAME parser_test COMMAND parser_test)

## PARSER UTILS TEST
add_executable(
        parser_utils_test
        ${CTEST_SOURCE_DIRECTORY}/parser_utils_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser_utils.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(parser_utils_test check m)
add_test(NAME parser_utils_test COMMAND parser_utils_test)

## UTIL TEST
add_executable(
        util_test
        ${CTEST_SOURCE_DIRECTORY}/util_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/util.c)
target_link_libraries(util_test check m)
add_test(NAME util_test COMMAND util_test)

## COMMANDS PARSER TEST
add_executable(
        commands_parser_test
        ${CTEST_SOURCE_DIRECTORY}/commands_parser_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/logger.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/emalloc.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/buffer.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser_utils.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(commands_parser_test check m)
add_test(NAME commands_parser_test COMMAND commands_parser_test)

## USERS TEST
add_executable(
        users_test
        ${CTEST_SOURCE_DIRECTORY}/users_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util/users.c)
target_link_libraries(users_test check m)
add_test(NAME users_test COMMAND users_test)
