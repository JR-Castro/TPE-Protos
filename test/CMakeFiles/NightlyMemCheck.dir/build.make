# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nrossi/workspace/protos/TPE-Protos

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nrossi/workspace/protos/TPE-Protos

# Utility rule file for NightlyMemCheck.

# Include the progress variables for this target.
include test/CMakeFiles/NightlyMemCheck.dir/progress.make

test/CMakeFiles/NightlyMemCheck:
	cd /home/nrossi/workspace/protos/TPE-Protos/test && /usr/bin/ctest -D NightlyMemCheck

NightlyMemCheck: test/CMakeFiles/NightlyMemCheck
NightlyMemCheck: test/CMakeFiles/NightlyMemCheck.dir/build.make

.PHONY : NightlyMemCheck

# Rule to build all files generated by this target.
test/CMakeFiles/NightlyMemCheck.dir/build: NightlyMemCheck

.PHONY : test/CMakeFiles/NightlyMemCheck.dir/build

test/CMakeFiles/NightlyMemCheck.dir/clean:
	cd /home/nrossi/workspace/protos/TPE-Protos/test && $(CMAKE_COMMAND) -P CMakeFiles/NightlyMemCheck.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/NightlyMemCheck.dir/clean

test/CMakeFiles/NightlyMemCheck.dir/depend:
	cd /home/nrossi/workspace/protos/TPE-Protos && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nrossi/workspace/protos/TPE-Protos /home/nrossi/workspace/protos/TPE-Protos/test /home/nrossi/workspace/protos/TPE-Protos /home/nrossi/workspace/protos/TPE-Protos/test /home/nrossi/workspace/protos/TPE-Protos/test/CMakeFiles/NightlyMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/NightlyMemCheck.dir/depend

