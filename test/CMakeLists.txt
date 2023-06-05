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

## QUEUE TEST
add_executable(
        queue_test
        ${CTEST_SOURCE_DIRECTORY}/queue_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/logger.c
        ${CTEST_SOURCE_DIRECTORY}/../src/emalloc.c
        ${CTEST_SOURCE_DIRECTORY}/../src/queue.c)
target_link_libraries(queue_test check subunit rt m)
add_test(NAME queue_test COMMAND queue_test)

## PARSER TEST
add_executable(
        parser_test
        ${CTEST_SOURCE_DIRECTORY}/parser_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(parser_test check subunit rt m)
add_test(NAME parser_test COMMAND parser_test)

## PARSER UTILS TEST
add_executable(
        parser_utils_test
        ${CTEST_SOURCE_DIRECTORY}/parser_utils_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser_utils.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(parser_utils_test check subunit rt m)
add_test(NAME parser_utils_test COMMAND parser_utils_test)

## UTIL TEST
add_executable(
        util_test
        ${CTEST_SOURCE_DIRECTORY}/util_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util.c)
target_link_libraries(util_test check subunit rt m)
add_test(NAME util_test COMMAND util_test)

## COMMANDS PARSER TEST
add_executable(
        commands_parser_test
        ${CTEST_SOURCE_DIRECTORY}/commands_parser_test.c
        ${CTEST_SOURCE_DIRECTORY}/../src/util.c
        ${CTEST_SOURCE_DIRECTORY}/../src/logger.c
        ${CTEST_SOURCE_DIRECTORY}/../src/emalloc.c
        ${CTEST_SOURCE_DIRECTORY}/../src/queue.c
        ${CTEST_SOURCE_DIRECTORY}/../src/buffer.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser_utils.c
        ${CTEST_SOURCE_DIRECTORY}/../src/parsers/parser.c)
target_link_libraries(commands_parser_test check subunit rt m)
add_test(NAME commands_parser_test COMMAND commands_parser_test)
