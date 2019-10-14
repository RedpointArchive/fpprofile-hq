# CMakeList.txt : CMake project for fpprofile, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.7)

add_executable (fpprofile "fpprofile.c" "fpprofile.h" "netcode.c" "netcode.h")

if(WIN32)
	target_link_libraries (fpprofile ${CMAKE_CURRENT_SOURCE_DIR}/sodium-release.lib)
	target_include_directories (fpprofile SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
else()
	if(APPLE)
		option(sodium_USE_STATIC_LIBS ON)
	endif()
	include(${CMAKE_CURRENT_SOURCE_DIR}/Findsodium.cmake)
	target_include_directories (fpprofile SYSTEM PUBLIC ${sodium_INCLUDE_DIR})
	target_link_libraries (fpprofile ${sodium_LIBRARY_RELEASE} "m")
endif()