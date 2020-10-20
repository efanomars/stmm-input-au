# Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   stmm-input-au-defs.cmake

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_INPUT_AU_MAJOR_VERSION 0)
set(STMM_INPUT_AU_MINOR_VERSION 8) # !-U-!
set(STMM_INPUT_AU_VERSION "${STMM_INPUT_AU_MAJOR_VERSION}.${STMM_INPUT_AU_MINOR_VERSION}.0")

# required stmm-input version
set(STMM_INPUT_AU_REQ_STMM_INPUT_MAJOR_VERSION 0)
set(STMM_INPUT_AU_REQ_STMM_INPUT_MINOR_VERSION 15) # !-U-!
set(STMM_INPUT_AU_REQ_STMM_INPUT_VERSION "${STMM_INPUT_AU_REQ_STMM_INPUT_MAJOR_VERSION}.${STMM_INPUT_AU_REQ_STMM_INPUT_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMINPUT    REQUIRED  stmm-input>=${STMM_INPUT_AU_REQ_STMM_INPUT_VERSION})
endif()

# include dirs
list(APPEND STMMINPUTAU_EXTRA_INCLUDE_DIRS  "${STMMINPUT_INCLUDE_DIRS}")

set(STMMI_TEMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../libstmm-input-au/include")

list(APPEND STMMINPUTAU_INCLUDE_DIRS  "${STMMI_TEMP_INCLUDE_DIR}")
list(APPEND STMMINPUTAU_INCLUDE_DIRS  "${STMMINPUTAU_EXTRA_INCLUDE_DIRS}")

# libs
set(        STMMI_TEMP_EXTERNAL_LIBRARIES    "")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES    "${STMMINPUT_LIBRARIES}")

set(        STMMINPUTAU_EXTRA_LIBRARIES      "")
list(APPEND STMMINPUTAU_EXTRA_LIBRARIES      "${STMMI_TEMP_EXTERNAL_LIBRARIES}")

if (BUILD_SHARED_LIBS)
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-input-au/build/libstmm-input-au.so")
else()
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-input-au/build/libstmm-input-au.a")
endif()

list(APPEND STMMINPUTAU_LIBRARIES "${STMMI_LIB_FILE}")
list(APPEND STMMINPUTAU_LIBRARIES "${STMMINPUTAU_EXTRA_LIBRARIES}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    DefineAsSecondaryTarget(stmm-input-au  ${STMMI_LIB_FILE}  "${STMMINPUTAU_INCLUDE_DIRS}"  "" "${STMMI_TEMP_EXTERNAL_LIBRARIES}")
endif()
