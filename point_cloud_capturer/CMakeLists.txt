cmake_minimum_required(VERSION 3.20)

# Project
project(point_cloud_capturer LANGUAGES CXX VERSION 1.0.0)

# Build settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
# AUTO: enable if SDK is detected; ON: force; OFF: disable
set(ENABLE_K4A "AUTO" CACHE STRING "Enable Azure Kinect (k4a) support: AUTO|ON|OFF")
set_property(CACHE ENABLE_K4A PROPERTY STRINGS AUTO ON OFF)

# Dependencies
# 1) RealSense (required)
# Prefer modern CMake config; fall back to pkg-config if needed
find_package(realsense2 CONFIG QUIET)
if(NOT realsense2_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(REALSENSE2 REQUIRED IMPORTED_TARGET realsense2)
endif()

# 2) fast_float (header-only)
find_path(FAST_FLOAT_INCLUDE_DIR fast_float/fast_float.h)
if(NOT FAST_FLOAT_INCLUDE_DIR)
  message(FATAL_ERROR "fast_float headers not found. Provide via conda-forge (fast_float).")
endif()

# 3) Threads (portable)
find_package(Threads REQUIRED)

# 4) Azure Kinect (optional, per policy)
set(K4A_ENABLED FALSE)
if(ENABLE_K4A STREQUAL "ON")
  find_package(k4a REQUIRED)
  set(K4A_ENABLED TRUE)
elseif(ENABLE_K4A STREQUAL "AUTO")
  find_package(k4a QUIET)
  if(k4a_FOUND)
    set(K4A_ENABLED TRUE)
  endif()
endif()

if(NOT K4A_ENABLED)
  message(STATUS "Azure Kinect SDK not found or disabled: building WITHOUT Kinect support")
endif()

# Sources
# Start from all sources, then drop platform-specifics and Kinect if disabled
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Remove Windows-only dllmain on non-Windows
if(NOT WIN32)
  list(FILTER SOURCES EXCLUDE REGEX ".*/src/dllmain\\.cpp$")
endif()

# Exclude Kinect sources when not enabled
if(NOT K4A_ENABLED)
  # Remove both real-time and prerecorded Kinect sources
  list(FILTER SOURCES EXCLUDE REGEX ".*/src/kinect/.*\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX ".*/src/kinect_.*\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX ".*/src/prerecorded_kinect_.*\\.cpp$")
endif()

# Define the shared library
add_library(point_cloud_capturer SHARED ${SOURCES})

# Include directories
target_include_directories(point_cloud_capturer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include/multi_capturer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/external/plywoot
    ${FAST_FLOAT_INCLUDE_DIR}
)

# Link dependencies
if(realsense2_FOUND)
  target_link_libraries(point_cloud_capturer PRIVATE realsense2::realsense2)
else()
  target_link_libraries(point_cloud_capturer PRIVATE PkgConfig::REALSENSE2)
endif()

# Threads for any concurrency
target_link_libraries(point_cloud_capturer PRIVATE Threads::Threads)

# Azure Kinect (optional)
if(K4A_ENABLED)
  target_compile_definitions(point_cloud_capturer PRIVATE HAS_K4A)
  target_link_libraries(point_cloud_capturer PRIVATE ${k4a_LIBRARY} ${k4arecord_LIBRARY})
  target_include_directories(point_cloud_capturer PRIVATE ${k4a_INCLUDE_DIR})
endif()

# Strip any Windows-only libraries that were previously used as boilerplate
if(WIN32)
  # No implicit dependency on winmm or ws2_32 is present in the codebase.
  # Do not link them unless needed in the future.
endif()

# Visibility and soname
set_target_properties(point_cloud_capturer PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  OUTPUT_NAME "point_cloud_capturer"
)

# Installation
include(GNUInstallDirs)

# 1) Library
install(TARGETS point_cloud_capturer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}       # .dll on Windows
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}       # .so/.dylib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 2) Public headers — keep the C surface minimal for Unity
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/point_cloud_data.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/framework.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/point_cloud_capturer
)

# 3) Pkg-Config support (optional but handy)
# Generate a simple .pc for downstream if needed
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
  ${CMAKE_CURRENT_BINARY_DIR}/point_cloud_capturer.pc.in
  @ONLY
)

# Testing hook (optional): consumers can call `ctest` after build
enable_testing()
