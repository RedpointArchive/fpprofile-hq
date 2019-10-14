# CMakeList.txt : CMake project for fpprofile, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.7)

add_executable (fpprofile "fpprofile.c" "fpprofile.h" "netcode.c" "netcode.h")

add_dependencies (fpprofile sodium)
target_include_directories (fpprofile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../sodium/src/libsodium/include/)
target_link_libraries (fpprofile sodium)

if(UNIX)
	target_link_libraries (fpprofile "m")
endif()