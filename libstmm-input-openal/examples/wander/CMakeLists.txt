if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the wander source code and call cmake from there")
endif()

cmake_minimum_required(VERSION 3.0)

project(wander CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

include(FindPkgConfig)
find_package(PkgConfig)

# Version
set(WANDER_VERSION "0.9") # !-U-!

# Required libraries
set(WANDER_REQ_STMM_INPUT_OPENAL_VERSION "0.9") # !-U-!

# Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
pkg_check_modules(STMMINPUTOPENAL  REQUIRED  stmm-input-openal>=${WANDER_REQ_STMM_INPUT_OPENAL_VERSION})

pkg_check_modules(GTKMM            REQUIRED  gtkmm-3.0>=3.22.0)

list(APPEND WANDER_EXTRA_INCLUDE_DIRS   ${STMMINPUTOPENAL_INCLUDE_DIRS})
list(APPEND WANDER_EXTRA_INCLUDE_DIRS   ${GTKMM_INCLUDE_DIRS})
list(APPEND WANDER_EXTRA_LIBS           ${STMMINPUTOPENAL_LIBRARIES})
list(APPEND WANDER_EXTRA_LIBS           ${GTKMM_LIBRARIES})

# Add include directories of used libraries
include_directories(SYSTEM "${WANDER_EXTRA_INCLUDE_DIRS}")

# Source and headers files
set(WANDER_SOURCES
        ${PROJECT_SOURCE_DIR}/src/main.cc
        ${PROJECT_SOURCE_DIR}/src/wanderdata.h
        ${PROJECT_SOURCE_DIR}/src/wanderdata.cc
        ${PROJECT_SOURCE_DIR}/src/wanderdrawingarea.h
        ${PROJECT_SOURCE_DIR}/src/wanderdrawingarea.cc
        ${PROJECT_SOURCE_DIR}/src/wanderwindow.h
        ${PROJECT_SOURCE_DIR}/src/wanderwindow.cc
        )
# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

set(STMM_COMPILE_WARNINGS "-Wall -Wextra $ENV{STMM_CPP_OPTIONS}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	string(REPLACE "-Wsuggest-override" "" STMM_COMPILE_WARNINGS ${STMM_COMPILE_WARNINGS})
	string(REPLACE "-Wlogical-op" "" STMM_COMPILE_WARNINGS ${STMM_COMPILE_WARNINGS})
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${STMM_COMPILE_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STMM_COMPILE_WARNINGS}")

# Define executable
add_executable(wander ${WANDER_SOURCES})

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "wander was configured with the following options:")
message(STATUS " WANDER_SOURCES:                ${WANDER_SOURCES}")
message(STATUS " WANDER_DATA_FILES              ${WANDER_DATA_FILES}")
message(STATUS " WANDER_EXTRA_INCLUDE_DIRS:     ${WANDER_EXTRA_INCLUDE_DIRS}")
message(STATUS " WANDER_EXTRA_LIBS:             ${WANDER_EXTRA_LIBS}")
message(STATUS " CMAKE_BUILD_TYPE:               ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:          ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_FLAGS:                ${CMAKE_CXX_FLAGS}")
message(STATUS " CMAKE_CXX_FLAGS_DEBUG:          ${CMAKE_CXX_FLAGS_DEBUG}")
#message(STATUS " install prefix:                 ${CMAKE_INSTALL_PREFIX}")
endif()

target_link_libraries(wander ${WANDER_EXTRA_LIBS})

#install(TARGETS wander RUNTIME DESTINATION "bin")
