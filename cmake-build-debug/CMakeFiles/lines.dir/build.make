# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /var/lib/snapd/snap/clion/152/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /var/lib/snapd/snap/clion/152/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/user/Git/simple_gui

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/user/Git/simple_gui/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lines.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lines.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lines.dir/flags.make

CMakeFiles/lines.dir/tests/lines.c.o: CMakeFiles/lines.dir/flags.make
CMakeFiles/lines.dir/tests/lines.c.o: ../tests/lines.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/Git/simple_gui/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lines.dir/tests/lines.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lines.dir/tests/lines.c.o -c /home/user/Git/simple_gui/tests/lines.c

CMakeFiles/lines.dir/tests/lines.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lines.dir/tests/lines.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/user/Git/simple_gui/tests/lines.c > CMakeFiles/lines.dir/tests/lines.c.i

CMakeFiles/lines.dir/tests/lines.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lines.dir/tests/lines.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/user/Git/simple_gui/tests/lines.c -o CMakeFiles/lines.dir/tests/lines.c.s

# Object files for target lines
lines_OBJECTS = \
"CMakeFiles/lines.dir/tests/lines.c.o"

# External object files for target lines
lines_EXTERNAL_OBJECTS =

lines: CMakeFiles/lines.dir/tests/lines.c.o
lines: CMakeFiles/lines.dir/build.make
lines: libei.a
lines: ../_x11/libeibase64.a
lines: ../_x11/_x11.libextclass64.a
lines: CMakeFiles/lines.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/user/Git/simple_gui/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable lines"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lines.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lines.dir/build: lines

.PHONY : CMakeFiles/lines.dir/build

CMakeFiles/lines.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lines.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lines.dir/clean

CMakeFiles/lines.dir/depend:
	cd /home/user/Git/simple_gui/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/user/Git/simple_gui /home/user/Git/simple_gui /home/user/Git/simple_gui/cmake-build-debug /home/user/Git/simple_gui/cmake-build-debug /home/user/Git/simple_gui/cmake-build-debug/CMakeFiles/lines.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lines.dir/depend

