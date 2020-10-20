# Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
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

# File:   libstmm-input-au/test/CMakeLists.txt

include(CommonTesting)

option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    # Tests dir
    set(STMMI_TEST_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/test")

    # Test sources should end with .cxx, helper sources with .h .cc
    set(STMMI_AU_TEST_SOURCES
#             "${STMMI_TEST_SOURCES_DIR}/testFloGtkDeviceManager.cxx"
            )

    set(STMMI_AU_TEST_WITH_SOURCES
#             "${STMMI_TEST_SOURCES_DIR}/fakeflogtkbackend.h"
#             "${STMMI_TEST_SOURCES_DIR}/fakeflogtkbackend.cc"
#             "${STMMI_TEST_SOURCES_DIR}/fakeflogtkwindowdata.h"
#             "${STMMI_TEST_SOURCES_DIR}/fakeflogtkwindowdata.cc"
#             "${STMMI_TEST_SOURCES_DIR}/fakeflogtkdevicemanager.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureFloDM.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureGlibApp.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureTestBase.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixturevariantKeyRepeatMode.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixturevariantEventClasses.h"
            )

    TestFiles("${STMMI_AU_TEST_SOURCES}"
              "${STMMI_SOURCES};${STMMI_AU_TEST_WITH_SOURCES}"
              "${STMMINPUTAU_EXTRA_INCLUDE_DIRS}" "${STMMINPUTAU_EXTRA_LIBRARIES}" TRUE)

    include(CTest)
endif()
