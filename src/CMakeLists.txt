cmake_minimum_required(VERSION 3.0)
project(TPE-Protos C)

message("=== src/CMake")
# Shared variables
load_cache(${CMAKE_CURRENT_SOURCE_DIR}/ ../CMakeCache.txt)

# Include directories
include_directories("${PROJECT_ROOT_DIR}/src/include/}")
message(STATUS "INCLUDE_DIRS: ${INCLUDE_DIRS}")

# Main executable
file(GLOB_RECURSE SOURCES "*.c" "parsers/*.c")
message(STATUS "SOURCES: ${SOURCES}")
add_executable(pop3server ${SOURCES})
target_link_libraries(pop3server pthread)