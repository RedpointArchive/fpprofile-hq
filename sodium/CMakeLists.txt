# CMakeList.txt : CMake project for fpprofile, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.7)

file (GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/libsodium/*.c)
file (GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/libsodium/*.h)

add_library (sodium STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories (sodium PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/libsodium/include/sodium/)
target_compile_definitions (sodium PUBLIC "SODIUM_STATIC" "SODIUM_EXPORT=" "CONFIGURED=1")

if(UNIX)
elseif(MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
endif()