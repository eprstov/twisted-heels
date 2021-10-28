cmake_minimum_required(VERSION 3.17)
project(mixer)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

file( GLOB_RECURSE SOURCES LIST_DIRECTORIES false src/*.h src/*.cpp src/*.c )

find_file( UNISTD_H_PATH NAMES unistd.h )
if(UNISTD_H_PATH)
else()
	message( SEND_ERROR "unistd.h is required" )
endif()

include_directories( include/ )

find_package( PkgConfig REQUIRED )
pkg_check_modules( LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswscale
)

add_executable( mixer ${SOURCES} )
target_link_libraries( mixer PUBLIC PkgConfig::LIBAV )

source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX \\ FILES ${SOURCES} )

