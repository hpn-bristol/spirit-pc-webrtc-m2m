cmake_minimum_required(VERSION 3.20)

project(mdc_encoder LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# Portable install directory variables on all OSs
include(GNUInstallDirs)  # defines CMAKE_INSTALL_LIBDIR/BINDIR/INCLUDEDIR

# ---- Sources ----
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Windows-only DllMain: do not compile it on non-Windows
if(NOT WIN32)
  list(FILTER SOURCES EXCLUDE REGEX ".*/dllmain\\.cpp$")
endif()

add_library(mdc_encoder SHARED ${SOURCES})

target_include_directories(mdc_encoder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---- Threads (portable) ----
# enables -pthread on Unix, no-op elsewhere
find_package(Threads REQUIRED)
target_link_libraries(mdc_encoder PRIVATE Threads::Threads)

# ---- Draco via pkg-config (short & robust) ----
# We deliberately use the *variable* mode (no IMPORTED_TARGET) to avoid
# CMake rejecting relative INTERFACE include dirs from third-party .pc files.
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRACO REQUIRED draco)
# sets DRACO_PREFIX, *_LIBRARIES, *_CFLAGS_OTHER, etc.

# Use the package prefix directly; this guarantees an *absolute* include path
# and avoids any accidental "-Iinclude" that some .pc files may emit.
target_include_directories(mdc_encoder PRIVATE "${DRACO_PREFIX}/include")

# Link libraries and propagate non-include flags from pkg-config
target_link_libraries(mdc_encoder PRIVATE ${DRACO_LIBRARIES})
target_compile_options(mdc_encoder PRIVATE ${DRACO_CFLAGS_OTHER})
target_link_options(mdc_encoder PRIVATE ${DRACO_LDFLAGS_OTHER})

# ---- Install (standard layout, relocatable) ----
install(TARGETS mdc_encoder
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # .dll on Windows
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .so/.dylib on Linux/macOS
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mdc_encoder)

# ---- CTest: minimal API smoke test ----
include(CTest)  # defines BUILD_TESTING and calls enable_testing() by default
if(BUILD_TESTING)
  set(_pub_inc "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(MDC_TESTS
    api_smoke
    decode_smoke
    decode_positive
    idempotent_cleanup
  )

  foreach(test_name IN LISTS MDC_TESTS)
    set(test_src "${CMAKE_CURRENT_SOURCE_DIR}/tests/${test_name}.cpp")

    add_executable(${test_name} "${test_src}")
    target_include_directories(${test_name} PRIVATE "${_pub_inc}")
    target_link_libraries(${test_name} PRIVATE mdc_encoder)

    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
  endforeach()

  target_include_directories(decode_positive PRIVATE ${DRACO_INCLUDE_DIRS})
  target_link_libraries(decode_positive PRIVATE ${DRACO_LIBRARIES})
  target_compile_options(decode_positive PRIVATE ${DRACO_CFLAGS_OTHER})
  target_link_options(decode_positive PRIVATE ${DRACO_LDFLAGS_OTHER})

  add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${MDC_TESTS}
  )
endif()
