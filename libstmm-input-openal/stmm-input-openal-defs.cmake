# File: libstmm-input-openal/stmm-input-openal-defs.cmake

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

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_INPUT_OPENAL_MAJOR_VERSION 0)
set(STMM_INPUT_OPENAL_MINOR_VERSION 7) # !-U-!
set(STMM_INPUT_OPENAL_VERSION "${STMM_INPUT_OPENAL_MAJOR_VERSION}.${STMM_INPUT_OPENAL_MINOR_VERSION}.0")

# required stmm-input-au version
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_AU_MAJOR_VERSION "0")
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_AU_MINOR_VERSION "7") # !-U-!
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_AU_VERSION "${STMM_INPUT_OPENAL_REQ_STMM_INPUT_AU_MAJOR_VERSION}.${STMM_INPUT_OPENAL_REQ_STMM_INPUT_AU_MINOR_VERSION}")

# required stmm-input-ev version
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_MAJOR_VERSION "0")
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_MINOR_VERSION "14") # !-U-!
set(STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_VERSION "${STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_MAJOR_VERSION}.${STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_MINOR_VERSION}")

# required alure version
set(STMM_INPUT_OPENAL_REQ_ALURE_VERSION "1.2")
# required glibmm-2.4 version
set(STMM_INPUT_OPENAL_REQ_GLIBMM_VERSION "2.50.0")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMINPUTEV   REQUIRED  stmm-input-ev>=${STMM_INPUT_OPENAL_REQ_STMM_INPUT_EV_VERSION})
    pkg_check_modules(ALURE         REQUIRED  alure>=${STMM_INPUT_OPENAL_REQ_ALURE_VERSION})
    pkg_check_modules(GLIBMM        REQUIRED  glibmm-2.4>=${STMM_INPUT_OPENAL_REQ_GLIBMM_VERSION})
endif()

include("${PROJECT_SOURCE_DIR}/../libstmm-input-au/stmm-input-au-defs.cmake")

# include dirs
list(APPEND STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS  "${STMMINPUTAU_INCLUDE_DIRS}")
list(APPEND STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS  "${STMMINPUTEV_INCLUDE_DIRS}")
list(APPEND STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS  "${ALURE_INCLUDE_DIRS}")
list(APPEND STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS  "${GLIBMM_INCLUDE_DIRS}")

set(STMMI_TEMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../libstmm-input-openal/include")

list(APPEND STMMINPUTOPENAL_INCLUDE_DIRS  "${STMMI_TEMP_INCLUDE_DIR}")
list(APPEND STMMINPUTOPENAL_INCLUDE_DIRS  "${STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS}")

# libs
set(        STMMI_TEMP_EXTERNAL_LIBRARIES        "")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES        "${STMMINPUTEV_LIBRARIES}")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES        "${ALURE_LIBRARIES}")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES        "${GLIBMM_LIBRARIES}")

set(        STMMINPUTOPENAL_EXTRA_LIBRARIES      "")
list(APPEND STMMINPUTOPENAL_EXTRA_LIBRARIES      "${STMMINPUTAU_LIBRARIES}")
list(APPEND STMMINPUTOPENAL_EXTRA_LIBRARIES      "${STMMI_TEMP_EXTERNAL_LIBRARIES}")

if (BUILD_SHARED_LIBS)
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-input-openal/build/libstmm-input-openal.so")
else()
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-input-openal/build/libstmm-input-openal.a")
endif()

list(APPEND STMMINPUTOPENAL_LIBRARIES "${STMMI_LIB_FILE}")
list(APPEND STMMINPUTOPENAL_LIBRARIES "${STMMINPUTOPENAL_EXTRA_LIBRARIES}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    DefineAsSecondaryTarget(stmm-input-openal  ${STMMI_LIB_FILE}  "${STMMINPUTOPENAL_INCLUDE_DIRS}"  "stmm-input-au" "${STMMI_TEMP_EXTERNAL_LIBRARIES}")
endif()
