# File: libstmm-input-au/CMakeLists.txt

#  Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, see <http://www.gnu.org/licenses/>

cmake_minimum_required(VERSION 3.0)

project(stmm-input-au CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions("c++14")

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-input-au")

set(STMMI_HEADERS
#         "${STMMI_HEADERS_DIR}/stmm-input-au.h"
        "${STMMI_HEADERS_DIR}/playbackcapability.h"
        "${STMMI_HEADERS_DIR}/sndfinishedevent.h"
        "${STMMI_HEADERS_DIR}/sndmgmtcapability.h"
        "${STMMI_HEADERS_DIR}/stmm-input-au-config.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
#         "${STMMI_SOURCES_DIR}/floatingsources.h"
        "${STMMI_SOURCES_DIR}/playbackcapability.cc"
        "${STMMI_SOURCES_DIR}/sndfinishedevent.cc"
        "${STMMI_SOURCES_DIR}/sndmgmtcapability.cc"
        )

# Define library
add_library(stmm-input-au ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-input-au-config.cc")

include("stmm-input-au-defs.cmake")

target_include_directories(stmm-input-au SYSTEM PUBLIC ${STMMINPUTAU_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-input-au BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-input-au>  # <prefix>/include/stmm-input-au
)

target_link_libraries(stmm-input-au ${STMMINPUTAU_EXTRA_LIBRARIES})

set_target_properties(stmm-input-au PROPERTIES  ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-input-au PROPERTIES
        VERSION ${STMM_INPUT_AU_VERSION}
        SOVERSION ${STMM_INPUT_AU_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-input-au-${STMM_INPUT_AU_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-input-au)

# Set version for stmm-input-au-config.cc.in
set(STMMI_PKG_VERSION "${STMM_INPUT_AU_VERSION}")
# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-input-au-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-au-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-input-au.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-au.pc" @ONLY)


if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-input-au was configured with the following options:")
message(STATUS " STMMI_HEADERS:               ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:               ${STMMI_SOURCES}")
message(STATUS " STMMI_AU_EXTRA_INCLUDE_DIRS: ${STMMI_AU_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMI_AU_EXTRA_LIBS:         ${STMMI_AU_EXTRA_LIBS}")
message(STATUS " CMAKE_BUILD_TYPE:            ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:       ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:  ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:             ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:              ${STMMI_AU_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                  ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:               ${BUILD_TESTING}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS) #  OR ("$ENV{STMM_DEBIAN_PACKAGING}" STREQUAL "ON")
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-input-au doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-input-au LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS}   DESTINATION "include/stmm-input-au")

install(FILES  "${PROJECT_BINARY_DIR}/stmm-input-au.pc"   DESTINATION "lib/pkgconfig")

