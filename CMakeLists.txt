# ============================================================================
# DaktLib - Overlay Module
# ============================================================================
# External overlay system for displaying information over game windows
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# ============================================================================
# Library Definition
# ============================================================================

add_library(DaktOverlay STATIC
    src/OverlayWindow.cpp
    src/HotkeyManager.cpp
    src/ProcessDetector.cpp
    src/WindowTracker.cpp
    src/ScreenCapture.cpp
    src/Panel.cpp
)

add_library(Dakt::Overlay ALIAS DaktOverlay)

# ============================================================================
# Include Directories
# ============================================================================

target_include_directories(DaktOverlay
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Dependencies
# ============================================================================

target_link_libraries(DaktOverlay
    PUBLIC
        Dakt::Core
    PRIVATE
        Dakt::Logger
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(DaktOverlay
        PRIVATE
            dwmapi      # Desktop Window Manager
            d3d11       # Direct3D 11 (for DXGI capture)
            dxgi        # DXGI for desktop duplication
    )
endif()

# ============================================================================
# Compiler Settings
# ============================================================================

target_compile_features(DaktOverlay PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(DaktOverlay PRIVATE
        /W4             # Warning level 4
        /WX             # Warnings as errors
        /permissive-    # Strict conformance
        /utf-8          # UTF-8 source and execution
        /Zc:__cplusplus # Correct __cplusplus macro
    )
else()
    target_compile_options(DaktOverlay PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# ============================================================================
# Platform Requirements
# ============================================================================

if(NOT WIN32)
    message(WARNING "DaktOverlay is Windows-only, building stub implementation")
    target_compile_definitions(DaktOverlay PRIVATE DAKT_OVERLAY_STUB=1)
endif()

# ============================================================================
# Export Configuration
# ============================================================================

install(TARGETS DaktOverlay
    EXPORT DaktLibTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)
