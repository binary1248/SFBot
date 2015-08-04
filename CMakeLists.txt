cmake_minimum_required( VERSION 2.8 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE )
endif()

project( SFBot )

set( SFNUL_STATIC_LIBRARIES TRUE CACHE BOOL "Choose the link type for SFNUL" )

if( MSVC OR CMAKE_COMPILER_IS_GNUCXX )
	set( SFBot_USE_STATIC_STD_LIBS TRUE CACHE BOOL "True to link the runtime library statically, false to link them dynamically." )
endif()

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} )

find_package( SFNUL REQUIRED )

if( APPLE )
	find_library( COREFOUNDATION_LIBRARY CoreFoundation )
	mark_as_advanced( COREFOUNDATION_LIBRARY )
endif()

set( SRC_DIR "${PROJECT_SOURCE_DIR}/src" )
set( MODULES_DIR "${SRC_DIR}/modules" )

include( "${MODULES_DIR}/CMakeLists.txt" )

set(
	SOURCES
	${MODULES}
	"${SRC_DIR}/core_module.cpp"
	"${SRC_DIR}/core_module.hpp"
	"${SRC_DIR}/module.cpp"
	"${SRC_DIR}/module.hpp"
	"${SRC_DIR}/main.cpp"
)

include_directories( ${SRC_DIR} )
include_directories( ${MODULES_DIR} )
include_directories( SYSTEM ${SFNUL_INCLUDE_DIR} )

if( APPLE )
	include_directories( /System/Library/Frameworks/CoreFoundation.framework/Headers )
endif()

set( GNUCXX_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wunused-parameter -Wno-long-long -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )

add_executable( SFBot ${SOURCES} )

if( SFBot_USE_STATIC_STD_LIBS )
	if( MSVC )
		foreach( flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
			if( ${flag} MATCHES "/MD" )
				string( REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}" )
			endif()
		endforeach()
	elseif( CMAKE_COMPILER_IS_GNUCXX )
		set( GNULD_FLAGS "-static -static-libgcc -static-libstdc++" )
		set_target_properties( SFBot PROPERTIES LINK_FLAGS ${GNULD_FLAGS} )
	endif()
endif()

if( CMAKE_COMPILER_IS_GNUCXX )
	if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
		set( GNUCXX_FLAGS "-stdlib=libc++ ${GNUCXX_FLAGS}" )
	else()
		set( GNUCXX_FLAGS "-std=c++11 ${GNUCXX_FLAGS}" )
	endif()
	
	set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS ${GNUCXX_FLAGS} )
endif()

target_link_libraries( SFBot ${SFNUL_LIBRARY} ${SFNUL_DEPENDENCIES} )

install(
	TARGETS SFBot
	RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT SFBot
)