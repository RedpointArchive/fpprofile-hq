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
elif(MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
endif()

# defines { "SODIUM_STATIC", "SODIUM_EXPORT=" }
# 	includedirs { "sodium/src/libsodium/include/sodium/" }
# 	files {
# 		"sodium/src/libsodium/**.c",
# 		"sodium/src/libsodium/**.h",
# 	}
# 	defines { "CONFIGURED=1" }
# 	filter { "toolset:gcc" }
# 		disablewarnings{ "unused-function", "unused-parameter", "unused-variable", "unknown-pragmas", "type-limits", "unused-but-set-variable" }
# 		buildoptions { "-fPIC" }
# 	filter { "toolset:clang" }
# 		disablewarnings{ "unused-function", "unused-parameter", "unused-variable", "unknown-pragmas", "type-limits" }
# 		buildoptions { "-fPIC" }
# 	filter "system:windows"
# 		disablewarnings { "4244", "4715", "4197", "4146", "4324", "4456", "4100", "4459", "4245" }
# 		linkoptions { "/ignore:4221" }
# 	filter "system:not windows"
# 		targetname "sodium-%{cfg.platform}-%{cfg.buildcfg}"
# 	filter "options:not no-avx"
# 		vectorextensions "AVX"
# 	filter { "action:vs2010"}
# 		defines { "inline=__inline;NATIVE_LITTLE_ENDIAN;_CRT_SECURE_NO_WARNINGS;" }