cmake_minimum_required(VERSION 3.0)
project(TPE-Protos C)

message("=== src/CMake")
# Shared variables
load_cache(${CMAKE_CURRENT_SOURCE_DIR}/ ../CMakeCache.txt)

# Include directories
file(GLOB_RECURSE INCLUDE_DIRS "${PROJECT_ROOT_DIR}/src/include/*.h")
include_directories(${INCLUDE_DIRS})
message(STATUS "INCLUDE_DIRS: ${INCLUDE_DIRS}")

# Main executable
file(GLOB_RECURSE SOURCES "${PROJECT_ROOT_DIR}/src/*.c")
message(STATUS "SOURCES: ${SOURCES}")
add_executable(pop3server ${SOURCES})
target_link_libraries(pop3server pthread sctp)