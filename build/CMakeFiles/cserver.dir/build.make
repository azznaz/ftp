# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

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
CMAKE_SOURCE_DIR = /home/scutech/azznaz/ftp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/scutech/azznaz/ftp/build

# Include any dependencies generated for this target.
include CMakeFiles/cserver.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/cserver.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/cserver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cserver.dir/flags.make

CMakeFiles/cserver.dir/Cmd_Handle.cpp.o: CMakeFiles/cserver.dir/flags.make
CMakeFiles/cserver.dir/Cmd_Handle.cpp.o: /home/scutech/azznaz/ftp/Cmd_Handle.cpp
CMakeFiles/cserver.dir/Cmd_Handle.cpp.o: CMakeFiles/cserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/scutech/azznaz/ftp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cserver.dir/Cmd_Handle.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cserver.dir/Cmd_Handle.cpp.o -MF CMakeFiles/cserver.dir/Cmd_Handle.cpp.o.d -o CMakeFiles/cserver.dir/Cmd_Handle.cpp.o -c /home/scutech/azznaz/ftp/Cmd_Handle.cpp

CMakeFiles/cserver.dir/Cmd_Handle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cserver.dir/Cmd_Handle.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/scutech/azznaz/ftp/Cmd_Handle.cpp > CMakeFiles/cserver.dir/Cmd_Handle.cpp.i

CMakeFiles/cserver.dir/Cmd_Handle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cserver.dir/Cmd_Handle.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/scutech/azznaz/ftp/Cmd_Handle.cpp -o CMakeFiles/cserver.dir/Cmd_Handle.cpp.s

CMakeFiles/cserver.dir/server.cpp.o: CMakeFiles/cserver.dir/flags.make
CMakeFiles/cserver.dir/server.cpp.o: /home/scutech/azznaz/ftp/server.cpp
CMakeFiles/cserver.dir/server.cpp.o: CMakeFiles/cserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/scutech/azznaz/ftp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/cserver.dir/server.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cserver.dir/server.cpp.o -MF CMakeFiles/cserver.dir/server.cpp.o.d -o CMakeFiles/cserver.dir/server.cpp.o -c /home/scutech/azznaz/ftp/server.cpp

CMakeFiles/cserver.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cserver.dir/server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/scutech/azznaz/ftp/server.cpp > CMakeFiles/cserver.dir/server.cpp.i

CMakeFiles/cserver.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cserver.dir/server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/scutech/azznaz/ftp/server.cpp -o CMakeFiles/cserver.dir/server.cpp.s

CMakeFiles/cserver.dir/Ftp_Server.cpp.o: CMakeFiles/cserver.dir/flags.make
CMakeFiles/cserver.dir/Ftp_Server.cpp.o: /home/scutech/azznaz/ftp/Ftp_Server.cpp
CMakeFiles/cserver.dir/Ftp_Server.cpp.o: CMakeFiles/cserver.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/scutech/azznaz/ftp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/cserver.dir/Ftp_Server.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cserver.dir/Ftp_Server.cpp.o -MF CMakeFiles/cserver.dir/Ftp_Server.cpp.o.d -o CMakeFiles/cserver.dir/Ftp_Server.cpp.o -c /home/scutech/azznaz/ftp/Ftp_Server.cpp

CMakeFiles/cserver.dir/Ftp_Server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cserver.dir/Ftp_Server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/scutech/azznaz/ftp/Ftp_Server.cpp > CMakeFiles/cserver.dir/Ftp_Server.cpp.i

CMakeFiles/cserver.dir/Ftp_Server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cserver.dir/Ftp_Server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/scutech/azznaz/ftp/Ftp_Server.cpp -o CMakeFiles/cserver.dir/Ftp_Server.cpp.s

# Object files for target cserver
cserver_OBJECTS = \
"CMakeFiles/cserver.dir/Cmd_Handle.cpp.o" \
"CMakeFiles/cserver.dir/server.cpp.o" \
"CMakeFiles/cserver.dir/Ftp_Server.cpp.o"

# External object files for target cserver
cserver_EXTERNAL_OBJECTS =

cserver: CMakeFiles/cserver.dir/Cmd_Handle.cpp.o
cserver: CMakeFiles/cserver.dir/server.cpp.o
cserver: CMakeFiles/cserver.dir/Ftp_Server.cpp.o
cserver: CMakeFiles/cserver.dir/build.make
cserver: CMakeFiles/cserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/scutech/azznaz/ftp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable cserver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cserver.dir/build: cserver
.PHONY : CMakeFiles/cserver.dir/build

CMakeFiles/cserver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cserver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cserver.dir/clean

CMakeFiles/cserver.dir/depend:
	cd /home/scutech/azznaz/ftp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/scutech/azznaz/ftp /home/scutech/azznaz/ftp /home/scutech/azznaz/ftp/build /home/scutech/azznaz/ftp/build /home/scutech/azznaz/ftp/build/CMakeFiles/cserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cserver.dir/depend

