# stmm-input-gtk-xi: test/CMakeLists.txt

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

include(CommonTesting)

option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    # Tests dir
    set(STMMI_TEST_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/test")

    # Test sources should end with .cxx, helper sources with .h .cc
    set(STMMI_OPENAL_TEST_SOURCES
#             "${STMMI_TEST_SOURCES_DIR}/testOpenAlDeviceManager.cxx"
            )

    set(STMMI_OPENAL_TEST_WITH_SOURCES
#             "${STMMI_TEST_SOURCES_DIR}/fakeopenalbackend.h"
#             "${STMMI_TEST_SOURCES_DIR}/fakeopenalbackend.cc"
#             "${STMMI_TEST_SOURCES_DIR}/fakeopenalwindowdata.h"
#             "${STMMI_TEST_SOURCES_DIR}/fakeopenalwindowdata.cc"
#             "${STMMI_TEST_SOURCES_DIR}/fakeopenaldevicemanager.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureAlDM.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureGlibApp.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixtureTestBase.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixturevariantKeyRepeatMode.h"
#             "${STMMI_TEST_SOURCES_DIR}/fixturevariantEventClasses.h"
            )

    TestFiles("${STMMI_OPENAL_TEST_SOURCES}"
              "${STMMI_SOURCES};${STMMI_OPENAL_TEST_WITH_SOURCES}"
              "${STMMINPUTOPENAL_EXTRA_INCLUDE_DIRS}" "${STMMINPUTOPENAL_EXTRA_LIBRARIES}" TRUE)

    include(CTest)
endif()
