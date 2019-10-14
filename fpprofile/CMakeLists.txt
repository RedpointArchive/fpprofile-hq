# CMakeList.txt : CMake project for fpprofile, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Add source to this project's executable.
add_executable (fpprofile "fpprofile.c" "fpprofile.h" "netcode.c" "netcode.h" "sodium.h")

# TODO: Add tests and install targets if needed.
target_link_libraries (fpprofile "sodium-release.lib")
target_link_directories (fpprofile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (fpprofile SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})