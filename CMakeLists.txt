cmake_minimum_required(VERSION 3.7)
project(stream-anonymization)

# inspired by on https://github.com/cpplibv/libv

include(ExternalProject)

set(CMAKE_VERBOSE_MAKEFILE on)
#set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")


set(PATH_PRJ ${CMAKE_CURRENT_SOURCE_DIR})
set(PATH_SRC ${PATH_PRJ}/src)
set(PATH_EXT ${PATH_PRJ}/ext)
set(PATH_EXT_SRC ${PATH_PRJ}/ext_src)


# Variables ----------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_compile_options(/Wall)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Weverything
                -Wno-comment
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
        )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic -Wno-comment
		-Wcast-qual
		-Wdelete-non-virtual-dtor
		-Wdouble-promotion
		-Wduplicated-cond
		-Wlogical-op
		-Wnon-virtual-dtor
		-Wnull-dereference
		-Wold-style-cast
		-Wsuggest-override
		-Wundef
#		-Wuseless-cast
# 		-Wconversion
# 		-Wfloat-equal

# Only enable time-to-time to check on what is going on:
# 		-Winline
# 		-Wsuggest-final-methods
# 		-Wsuggest-final-types
		)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
		add_compile_options(
			-Wduplicated-branches
			-Wrestrict
			-Wshadow-compatible-local
		)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
		# Sanitizers current only available on unix
		# add_compile_options(-fsanitize=address)
		# add_compile_options(-fsanitize=kernel-address)
		# add_compile_options(-fsanitize=leak)
		# add_compile_options(-fsanitize=thread)
		# add_compile_options(-fsanitize=undefined)
		# link_libraries(ubsan)
		# link_libraries(asan)
	endif()
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-Og)
	add_compile_options(-ggdb3)
	add_compile_options(-fno-omit-frame-pointer)

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-Ofast)
	add_compile_options(-flto)
#	SET(CMAKE_AR "gcc-ar")
#	SET(CMAKE_NM "gcc-nm")
#	SET(CMAKE_RANLIB "gcc-ranlib")
	set(USE_STATIC_LINK -static)
else()
	message(WARNING "Failed to identify [${CMAKE_BUILD_TYPE}] as a build type")
endif()

add_definitions(-m64)

if(NOT WIN32)
	link_libraries(pthread)
endif()

set(GROUP_EXTERNAL_PROJECT)

set(TOOLSET)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(TOOLSET msvc)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(TOOLSET clang)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(TOOLSET gcc)
endif()



list(APPEND GROUP_EXTERNAL_PROJECT ext_range)
ExternalProject_Add(ext_range
	GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
	GIT_TAG master
	CONFIGURE_COMMAND ""
	BUILD_COMMAND "" # range is header only, skip tests
	PREFIX ${PATH_EXT_SRC}/range
	INSTALL_COMMAND
		COMMAND mkdir -p ${PATH_EXT}/range/
		COMMAND cp -r -f ${PATH_EXT_SRC}/range/src/ext_range/include/ ${PATH_EXT}/range/
	#EXCLUDE_FROM_ALL 1
)

list(APPEND GROUP_EXTERNAL_PROJECT ext_date_h)
ExternalProject_Add(ext_date_h
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG master
	CONFIGURE_COMMAND ""
	BUILD_COMMAND "" # date.h is header only, skip tests
	PREFIX ${PATH_EXT_SRC}/date
	INSTALL_COMMAND
		COMMAND mkdir -p ${PATH_EXT}/date/
		COMMAND cp -r -f ${PATH_EXT_SRC}/date/src/ext_date_h/include/ ${PATH_EXT}/date/
	#EXCLUDE_FROM_ALL 1
)

list(APPEND GROUP_EXTERNAL_PROJECT ext_fmt)
ExternalProject_Add(ext_fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG master
	PREFIX ${PATH_EXT_SRC}/fmt
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${PATH_EXT}/fmt
		-DFMT_DOC=OFF
		-DFMT_TEST=OFF
	EXCLUDE_FROM_ALL 1
)

find_package(Threads)

include_directories(${PATH_INC})
include_directories(${PATH_SRC})
include_directories(SYSTEM ${PATH_EXT}/range/include)
include_directories(SYSTEM ${PATH_EXT}/date/include)
include_directories(SYSTEM ${PATH_EXT}/fmt/include)
link_directories(${PATH_EXT}/fmt/lib64)

add_executable(stream-anonymization ${PATH_SRC}/main.cpp)
add_dependencies(stream-anonymization ext_range ext_date_h ext_fmt)
target_link_libraries(stream-anonymization ${CMAKE_THREAD_LIBS_INIT} fmt)
