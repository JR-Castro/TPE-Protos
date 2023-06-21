cmake_minimum_required(VERSION 3.0)
project(TPE-Protos C)

message("=== src/CMake")
# Shared variables
load_cache(${CMAKE_CURRENT_SOURCE_DIR}/ ../CMakeCache.txt)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include/)

# Pop3 Server
file(GLOB_RECURSE SOURCES "util/*.c" "parsers/*.c" "manager/*.c" "selector_and_stm/*.c")
message(STATUS "SOURCES: ${SOURCES}")
add_executable(pop3server ${SOURCES} main.c pop3.c)
target_link_libraries(pop3server pthread)

# Management Client
set(MGMT_CLIENT_SOURCES
   ${PROJECT_SOURCE_DIR}/manager/manager_protocol.c manager_client.c
   ${PROJECT_SOURCE_DIR}/util/logger.c ${PROJECT_SOURCE_DIR}/util/emalloc.c)

# Add an executable target for the first executable
add_executable(managerclient ${MGMT_CLIENT_SOURCES})