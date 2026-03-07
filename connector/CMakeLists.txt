cmake_minimum_required(VERSION 3.20)
project(WebRTCConnector LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# --- Explicit allow-list to avoid building proxies and the unit test with main() ---
set(WEBRTCCONNECTOR_SOURCES
    plugin.cpp
    log.cpp
    webrtc_connection.cpp
    connected_client.cpp
    track_frame.cpp
    track_frame_queue.cpp
    track_internal.cpp
    pch.cpp
)
if (WIN32)
  list(APPEND WEBRTCCONNECTOR_SOURCES dllmain.cpp)
endif()

add_library(WebRTCConnector SHARED ${WEBRTCCONNECTOR_SOURCES})
target_include_directories(WebRTCConnector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(WebRTCConnector PRIVATE Threads::Threads)
if (WIN32)
  target_link_libraries(WebRTCConnector PRIVATE ws2_32)
endif()

# --- Generate a self-contained public header from plugin.h (+ any C ABI in log.h) ---
find_package(Python3 3.9 COMPONENTS Interpreter REQUIRED)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PUBLIC_HEADER_OUT "${GENERATED_DIR}/WebRTCConnector.h")

add_custom_command(
  OUTPUT  ${PUBLIC_HEADER_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
  COMMAND ${Python3_EXECUTABLE}
          ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_header.py
          --plugin ${CMAKE_CURRENT_SOURCE_DIR}/plugin.h
          --log    ${CMAKE_CURRENT_SOURCE_DIR}/log.h
          --out    ${PUBLIC_HEADER_OUT}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.h
          ${CMAKE_CURRENT_SOURCE_DIR}/log.h
          ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_header.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating self-contained WebRTCConnector.h (opaque types inferred on-the-fly)"
  VERBATIM
)
add_custom_target(gen_public_header ALL DEPENDS ${PUBLIC_HEADER_OUT})
add_dependencies(WebRTCConnector gen_public_header)

# --- Header self-containment smoke tests (C and C++) ---
enable_testing()

# Point tests to the repo sources you already have
set(SMOKE_C   "${CMAKE_CURRENT_SOURCE_DIR}/tests/header_c_smoke.c")
set(SMOKE_CPP "${CMAKE_CURRENT_SOURCE_DIR}/tests/header_cpp_smoke.cpp")

# Verify they exist to avoid 'no sources' configure errors
if(NOT EXISTS "${SMOKE_C}")
  message(FATAL_ERROR "Missing test source: ${SMOKE_C}")
endif()
if(NOT EXISTS "${SMOKE_CPP}")
  message(FATAL_ERROR "Missing test source: ${SMOKE_CPP}")
endif()

# C smoke target
add_executable(header_c_smoke "${SMOKE_C}")
add_dependencies(header_c_smoke gen_public_header)
target_include_directories(header_c_smoke PRIVATE "${GENERATED_DIR}")

# C++ smoke target
add_executable(header_cpp_smoke "${SMOKE_CPP}")
add_dependencies(header_cpp_smoke gen_public_header)
target_include_directories(header_cpp_smoke PRIVATE "${GENERATED_DIR}")

# Build-as-test pattern: compile each target as an individual CTest
add_test(
  NAME header_c_smoke_build
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target header_c_smoke
)
add_test(
  NAME header_cpp_smoke_build
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target header_cpp_smoke
)

# --- Install DLL/SO and generated header for pixi packaging ---
install(TARGETS WebRTCConnector
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES ${PUBLIC_HEADER_OUT}
        DESTINATION include/WebRTCConnector
        RENAME WebRTCConnector.h)
