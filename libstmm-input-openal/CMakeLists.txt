# File: libstmm-input-openal/CMakeLists.txt

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

project(stmm-input-openal CXX C)

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
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-input-openal")

set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/openaldevicemanager.h"
        #"${STMMI_HEADERS_DIR}/stmm-input-openal.h"
        "${STMMI_HEADERS_DIR}/stmm-input-openal-config.h"
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_HEADERS  ${STMMI_HEADERS}
        "${STMMI_HEADERS_DIR}/libmain.h"
        )
endif()
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/openalbackend.h"
        "${STMMI_SOURCES_DIR}/openalbackend.cc"
        "${STMMI_SOURCES_DIR}/openaldevicemanager.cc"
        "${STMMI_SOURCES_DIR}/openallistenerextradata.h"
        "${STMMI_SOURCES_DIR}/playbackdevice.h"
        "${STMMI_SOURCES_DIR}/playbackdevice.cc"
        "${STMMI_SOURCES_DIR}/recycler.h"
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_SOURCES
        ${STMMI_SOURCES}
        "${STMMI_SOURCES_DIR}/libmain.cc"
        )
endif()

# Define library
add_library(stmm-input-openal ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-input-openal-config.cc")

include("stmm-input-openal-defs.cmake")

target_include_directories(stmm-input-openal SYSTEM PUBLIC ${STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-input-openal BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-input-openal>  # <prefix>/include/stmm-input-openal
)

find_package(Threads REQUIRED)
target_link_libraries(stmm-input-openal ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(stmm-input-openal ${STMMINPUTOPENAL_EXTRA_LIBRARIES})

set_target_properties(stmm-input-openal PROPERTIES  ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-input-openal PROPERTIES
        VERSION ${STMM_INPUT_OPENAL_VERSION}
        SOVERSION ${STMM_INPUT_OPENAL_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-input-openal-${STMM_INPUT_OPENAL_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-input-openal)

# TODO only if plugin => create option COMPILE_AS_PLUGIN=ON (default)
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (BUILD_SHARED_LIBS))
    target_compile_options(stmm-input-openal PUBLIC "-Wl,-E")
endif()

# Set version for stmm-input-openal-config.cc.in
set(STMMI_PKG_VERSION "${STMM_INPUT_OPENAL_VERSION}")
set(STMMI_PKG_PLUGIN_NAME "90stmm-input-openal")
# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-input-openal-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-openal-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-input-openal.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-openal.pc" @ONLY)

include(GNUInstallDirs)
set(STMMI_PLUGINS_DATA_DIR      "${CMAKE_INSTALL_FULL_SYSCONFDIR}/stmm-input-dl")
set(STMMI_PLUGINS_USER_DATA_DIR ".local/${CMAKE_INSTALL_SYSCONFDIR}/stmm-input-dl") # not used

set(STMMI_OPENAL_INSTALL_LIB_PATH1 "${CMAKE_INSTALL_PREFIX}/lib/libstmm-input-openal.so.${STMM_INPUT_OPENAL_MAJOR_VERSION}")
set(STMMI_OPENAL_INSTALL_LIB_PATH2 "/usr/local/lib/libstmm-input-openal.so.${STMM_INPUT_OPENAL_MAJOR_VERSION}")
set(STMMI_OPENAL_INSTALL_LIB_PATH3 "/usr/lib/libstmm-input-openal.so.${STMM_INPUT_OPENAL_MAJOR_VERSION}")
set(STMMI_OPENAL_INSTALL_LIB_PATH4 "libstmm-input-openal-so.${STMM_INPUT_OPENAL_MAJOR_VERSION}")

configure_file("${PROJECT_SOURCE_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp.in"
               "${PROJECT_BINARY_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp" @ONLY)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-input-openal was configured with the following options:")
message(STATUS " STMMI_HEADERS:                   ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                   ${STMMI_SOURCES}")
message(STATUS " STMMI_OPENAL_EXTRA_INCLUDE_DIRS: ${STMMI_OPENAL_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMI_OPENAL_EXTRA_LIBS:         ${STMMI_OPENAL_EXTRA_LIBS}")
message(STATUS " CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:           ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:      ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                 ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                  ${STMMI_OPENAL_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                      ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                   ${BUILD_TESTING}")
if (BUILD_SHARED_LIBS)
message(STATUS " STMMI_PLUGINS_DATA_DIR:          ${STMMI_PLUGINS_DATA_DIR}")
message(STATUS " STMMI_PLUGINS_USER_DATA_DIR:     ${STMMI_PLUGINS_USER_DATA_DIR}")
endif()
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS) #  OR ("$ENV{STMM_DEBIAN_PACKAGING}" STREQUAL "ON")
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-input-openal doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-input-openal LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS}   DESTINATION "include/stmm-input-openal")

install(FILES  "${PROJECT_BINARY_DIR}/stmm-input-openal.pc"   DESTINATION "lib/pkgconfig")

if (BUILD_SHARED_LIBS)
    install(FILES  "${PROJECT_BINARY_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp"  DESTINATION "${STMMI_PLUGINS_DATA_DIR}")
endif()

