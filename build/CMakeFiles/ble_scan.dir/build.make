# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/abarcelos/miicare

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/abarcelos/miicare/build

# Include any dependencies generated for this target.
include CMakeFiles/ble_scan.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ble_scan.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ble_scan.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ble_scan.dir/flags.make

CMakeFiles/ble_scan.dir/src/ble_scan.c.o: CMakeFiles/ble_scan.dir/flags.make
CMakeFiles/ble_scan.dir/src/ble_scan.c.o: ../src/ble_scan.c
CMakeFiles/ble_scan.dir/src/ble_scan.c.o: CMakeFiles/ble_scan.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/abarcelos/miicare/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ble_scan.dir/src/ble_scan.c.o"
	/home/abarcelos/x-tools/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc --sysroot=/home/abarcelos/x-tools/arm-linux-gnueabihf/arm-linux-gnueabihf $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/ble_scan.dir/src/ble_scan.c.o -MF CMakeFiles/ble_scan.dir/src/ble_scan.c.o.d -o CMakeFiles/ble_scan.dir/src/ble_scan.c.o -c /home/abarcelos/miicare/src/ble_scan.c

CMakeFiles/ble_scan.dir/src/ble_scan.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ble_scan.dir/src/ble_scan.c.i"
	/home/abarcelos/x-tools/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc --sysroot=/home/abarcelos/x-tools/arm-linux-gnueabihf/arm-linux-gnueabihf $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/abarcelos/miicare/src/ble_scan.c > CMakeFiles/ble_scan.dir/src/ble_scan.c.i

CMakeFiles/ble_scan.dir/src/ble_scan.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ble_scan.dir/src/ble_scan.c.s"
	/home/abarcelos/x-tools/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc --sysroot=/home/abarcelos/x-tools/arm-linux-gnueabihf/arm-linux-gnueabihf $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/abarcelos/miicare/src/ble_scan.c -o CMakeFiles/ble_scan.dir/src/ble_scan.c.s

# Object files for target ble_scan
ble_scan_OBJECTS = \
"CMakeFiles/ble_scan.dir/src/ble_scan.c.o"

# External object files for target ble_scan
ble_scan_EXTERNAL_OBJECTS =

ble_scan: CMakeFiles/ble_scan.dir/src/ble_scan.c.o
ble_scan: CMakeFiles/ble_scan.dir/build.make
ble_scan: ../third_party/lib/libbluetooth.so.3.19.3
ble_scan: ../third_party/lib/libjson-c.so.5.1.0
ble_scan: CMakeFiles/ble_scan.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/abarcelos/miicare/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ble_scan"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ble_scan.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ble_scan.dir/build: ble_scan
.PHONY : CMakeFiles/ble_scan.dir/build

CMakeFiles/ble_scan.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ble_scan.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ble_scan.dir/clean

CMakeFiles/ble_scan.dir/depend:
	cd /home/abarcelos/miicare/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/abarcelos/miicare /home/abarcelos/miicare /home/abarcelos/miicare/build /home/abarcelos/miicare/build /home/abarcelos/miicare/build/CMakeFiles/ble_scan.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ble_scan.dir/depend
