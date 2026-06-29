cmake_minimum_required(VERSION 3.16)
project(clon3d-emulationstation VERSION 0.1.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# C++ standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Build type default
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(CLON3D_GLES   "Use OpenGL ES (for Raspberry Pi)"   OFF)
option(CLON3D_DEBUG  "Enable debug logging at compile time" ON)

if(CLON3D_DEBUG)
    add_compile_definitions(CLON3D_DEBUG_BUILD)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# SDL2
find_package(SDL2 REQUIRED)

# pugixml (header-only build via CMake FetchContent, or system package)
find_package(pugixml QUIET)
if(NOT pugixml_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml.git
        GIT_TAG        v1.14
    )
    FetchContent_MakeAvailable(pugixml)
endif()

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------

set(CORE_SOURCES
    src/core/Settings.cpp
    src/core/InputManager.cpp
)

set(COMPONENT_SOURCES
    src/components/CarouselComponent.cpp
)

set(VIEW_SOURCES
    src/views/SystemView.cpp
    src/views/ViewController.cpp
)

set(THEME_SOURCES
    src/theme/ThemeData.cpp
    src/theme/ThemeComponentFactory.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${COMPONENT_SOURCES}
    ${VIEW_SOURCES}
    ${THEME_SOURCES}
    src/main.cpp
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------

add_executable(clon3d-es ${ALL_SOURCES})

target_include_directories(clon3d-es PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(clon3d-es PRIVATE
    ${SDL2_LIBRARIES}
    pugixml
)

# Platform-specific
if(CLON3D_GLES)
    target_compile_definitions(clon3d-es PRIVATE CLON3D_USE_GLES)
    find_package(OpenGL REQUIRED)
    target_link_libraries(clon3d-es PRIVATE GLESv2 EGL)
else()
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        target_link_libraries(clon3d-es PRIVATE OpenGL::GL)
    endif()
endif()

# Raspberry Pi: link against broadcom libraries if present.
if(EXISTS "/opt/vc/lib/libbcm_host.so")
    target_include_directories(clon3d-es PRIVATE /opt/vc/include)
    target_link_directories(clon3d-es PRIVATE /opt/vc/lib)
    target_link_libraries(clon3d-es PRIVATE bcm_host)
    target_compile_definitions(clon3d-es PRIVATE CLON3D_RASPI)
endif()

# ---------------------------------------------------------------------------
# Compiler warnings
# ---------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(clon3d-es PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS clon3d-es
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY themes/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/clon3d-es/themes)

# ---------------------------------------------------------------------------
# Unit tests (optional, requires Catch2)
# ---------------------------------------------------------------------------
option(CLON3D_BUILD_TESTS "Build unit tests" OFF)

if(CLON3D_BUILD_TESTS)
    find_package(Catch2 3 QUIET)
    if(Catch2_FOUND)
        add_subdirectory(tests)
    else()
        message(STATUS "Catch2 not found — skipping tests")
    endif()
endif()

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Clon3D EmulationStation ===")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  OpenGL ES  : ${CLON3D_GLES}")
message(STATUS "  Debug build: ${CLON3D_DEBUG}")
message(STATUS "")
