# Copyright (C) 2026 Alfredo Tupone
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
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

cmake_minimum_required(VERSION 3.14)
project(ReactorAsterix VERSION 0.0.1 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# SYSTEM PATHS & INSTALLATION LOGIC
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------------------------------------------
# COMPILER SETTINGS
# ---------------------------------------------------------------------------
# Use C++17 for atomic support and modern syntax
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STRICT_WERROR "Treat warnings as errors" OFF)

# Add common compiler flags for safety and debugging
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wshadow
    -Wnon-virtual-dtor
    -Wno-interference-size
    -ggdb -O2
)

if(STRICT_WERROR)
    add_compile_options(-Werror)
endif()

# ---------------------------------------------------------------------------
# THE LIBRARY
# ---------------------------------------------------------------------------
# 1. Library Definition
# ------------------------------------------------------------------------------
# Gathering source files
set(LIB_HEADERS
    include/ReactorAsterix/core/AsterixCategoryHandler.h
    include/ReactorAsterix/core/AsterixConstants.h
    include/ReactorAsterix/core/AsterixDataItemHandlerExtendedLength.h
    include/ReactorAsterix/core/AsterixDataItemHandlerFixedLength.h
    include/ReactorAsterix/core/AsterixDataItemHandlerBase.h
    include/ReactorAsterix/core/AsterixDiagnostics.h
    include/ReactorAsterix/core/asterixExceptions.h
    include/ReactorAsterix/core/AsterixMessage.h
    include/ReactorAsterix/core/AsterixPacketHandler.h
    include/ReactorAsterix/core/IAsterixCategoryHandler.h
    include/ReactorAsterix/core/IAsterixDataItemHandler.h
    include/ReactorAsterix/core/SourceIdentifier.h
    include/ReactorAsterix/core/SourceStateManager.h
    include/ReactorAsterix/cat001/Asterix1DataItemCollection.h
    include/ReactorAsterix/cat001/Asterix1Handler.h
    include/ReactorAsterix/cat001/Asterix1Report.h
    include/ReactorAsterix/cat001/IAsterix1Listener.h
    include/ReactorAsterix/cat002/Asterix2DataItemCollection.h
    include/ReactorAsterix/cat002/Asterix2Handler.h
    include/ReactorAsterix/cat002/Asterix2Report.h
    include/ReactorAsterix/cat002/IAsterix2Listener.h
)

set(LIB_SOURCES
    src/core/AsterixPacketHandler.cc
    src/cat001/Asterix1DataItemCollection.cc
    src/cat001/Asterix1Handler.cc
    src/cat002/Asterix2DataItemCollection.cc
    src/cat002/Asterix2Handler.cc
)

# Create the SHARED library
add_library(ReactorAsterix SHARED ${LIB_SOURCES} ${LIB_HEADERS})

add_library(ReactorAsterix::ReactorAsterix ALIAS ReactorAsterix)

# 2. Include Directories
# ------------------------------------------------------------------------------
target_include_directories(ReactorAsterix PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set versioning for the shared object (standard for .so files)
set_target_properties(ReactorAsterix PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

# Install the library binary
install(TARGETS ReactorAsterix
    EXPORT ReactorAsterixTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers maintaining the folder structure
install(DIRECTORY include/ReactorAsterix
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install the CMake export file (for find_package support)
install(EXPORT ReactorAsterixTargets
    FILE ReactorAsterixTargets.cmake
    NAMESPACE ReactorAsterix::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ReactorAsterix
)

enable_testing()

find_package(GTest)

add_executable(unit_tests tests/test_cat001.cc)
target_link_libraries(unit_tests PRIVATE ReactorAsterix GTest::gtest_main)
add_test(NAME AllTests COMMAND unit_tests)

add_executable(asterix_pro_example examples/example_pro.cc)

target_link_libraries(asterix_pro_example PRIVATE ReactorAsterix)

# Set include paths for the example
target_include_directories(asterix_pro_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# C++17 Best Practices: Enable extra warnings and treat them as errors if needed
target_compile_options(asterix_pro_example PRIVATE -Wall -Wextra -Wpedantic)

# Convenience target: 'make run_example'
add_custom_target(run_example
    COMMAND asterix_pro_example
    DEPENDS asterix_pro_example
    COMMENT "Running the improved ASTERIX example..."
)

# Check if AtuReactor is available on the system
find_package(AtuReactor QUIET)

if(AtuReactor_FOUND)
    message(STATUS "AtuReactor found: enabling networking examples")

    add_executable(atu_asterix_receiver examples/atu_asterix_receiver.cc)

    # Link against your library and the networking library
    target_link_libraries(atu_asterix_receiver PRIVATE
        ReactorAsterix
        AtuReactor::AtuReactor
    )

    # Ensure include paths are correct
    target_include_directories(atu_asterix_receiver PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    message(WARNING "AtuReactor not found. Skipping networking examples.")
endif()
