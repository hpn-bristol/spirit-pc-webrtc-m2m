cmake_minimum_required(VERSION 3.20)

project(raw_encoder LANGUAGES C CXX)

# ---- Language standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# ---- Options ----
option(VENDOR_ZDEPTH "Vendor Zdepth internally (exception to no-vendoring rule)" ON)

# ---- Portable install dirs ----
include(GNUInstallDirs)

# ---- Sources ----
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Windows-only DllMain
if(NOT WIN32)
  list(FILTER SOURCES EXCLUDE REGEX ".*/dllmain\\.cpp$")
endif()

add_library(spirit_idlab_raw SHARED ${SOURCES})

target_include_directories(spirit_idlab_raw
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---- Threads (portable) ----
find_package(Threads REQUIRED)
target_link_libraries(spirit_idlab_raw PRIVATE Threads::Threads)

# ---- Third-party deps via pkg-config ----
find_package(PkgConfig REQUIRED)

pkg_check_modules(JPEG REQUIRED libturbojpeg)
pkg_check_modules(WEBP REQUIRED libwebp)
pkg_check_modules(ZSTD REQUIRED libzstd)

target_include_directories(spirit_idlab_raw PRIVATE
  "${JPEG_PREFIX}/include"
  "${WEBP_PREFIX}/include"
  "${ZSTD_PREFIX}/include"
)

target_link_libraries(spirit_idlab_raw PRIVATE
  ${JPEG_LIBRARIES}
  ${WEBP_LIBRARIES}
  ${ZSTD_LIBRARIES}
)

target_compile_options(spirit_idlab_raw PRIVATE
  ${JPEG_CFLAGS_OTHER}
  ${WEBP_CFLAGS_OTHER}
  ${ZSTD_CFLAGS_OTHER}
)

target_link_options(spirit_idlab_raw PRIVATE
  ${JPEG_LDFLAGS_OTHER}
  ${WEBP_LDFLAGS_OTHER}
  ${ZSTD_LDFLAGS_OTHER}
)

# ---- Zdepth (vendored via FetchContent) ----
if(VENDOR_ZDEPTH)
  include(FetchContent)

  FetchContent_Declare(
    zdepth
    GIT_REPOSITORY https://github.com/catid/Zdepth.git
    GIT_TAG abc01e7
  )

  FetchContent_MakeAvailable(zdepth)

  target_include_directories(spirit_idlab_raw PRIVATE
    "${zdepth_SOURCE_DIR}/src")
  target_link_libraries(spirit_idlab_raw PRIVATE zdepth)
  target_compile_definitions(spirit_idlab_raw PRIVATE RAW_ENCODER_ENABLE_ZDEPTH=1)
else()
  target_compile_definitions(spirit_idlab_raw PRIVATE RAW_ENCODER_ENABLE_ZDEPTH=0)
endif()

# ---- Install (Unity-ready, relocatable) ----
install(TARGETS spirit_idlab_raw
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # .dll on Windows
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .so/.dylib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/raw_encoder
)

# ---- Tests: minimal link-only smoke test ----
include(CTest)

# ---- Tests: minimal link-only smoke test ----
include(CTest)
if(BUILD_TESTING)
  add_executable(api_smoke tests/api_smoke.cpp)
  target_include_directories(api_smoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
  target_link_libraries(api_smoke PRIVATE spirit_idlab_raw)

  add_test(NAME api_smoke COMMAND $<TARGET_FILE:api_smoke>)

  add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS api_smoke
  )
endif()
