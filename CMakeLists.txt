cmake_minimum_required(VERSION 4.2.1)

project(DaktLibOverlay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(dakt_overlay
    src/core/Color.cpp
    src/core/Compositor.cpp
    src/core/Geometry.cpp
    src/core/InputRouter.cpp
    src/core/Layer.cpp
    src/core/LayerTree.cpp
    src/core/OverlayManager.cpp
    src/core/OverlayWindow.cpp
    src/core/Painter.cpp
    src/core/Surface.cpp
    src/c_api.cpp
)

target_include_directories(dakt_overlay PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(dakt_overlay PUBLIC cxx_std_23)

if(WIN32)
    target_sources(dakt_overlay PRIVATE src/platform/windows/WindowsSurface.cpp)
    target_compile_definitions(dakt_overlay PUBLIC DAKT_OVERLAY_PLATFORM_WINDOWS)
elseif(APPLE)
    enable_language(OBJCXX)
    target_sources(dakt_overlay PRIVATE src/platform/mac/MacSurface.mm)
    target_compile_definitions(dakt_overlay PUBLIC DAKT_OVERLAY_PLATFORM_MAC)
else()
    target_sources(dakt_overlay PRIVATE src/platform/linux/LinuxSurface.cpp)
    target_compile_definitions(dakt_overlay PUBLIC DAKT_OVERLAY_PLATFORM_LINUX)
endif()

if(MSVC)
    target_compile_options(dakt_overlay PRIVATE /W4 /permissive-)
else()
    target_compile_options(dakt_overlay PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(dakt_overlay PROPERTIES POSITION_INDEPENDENT_CODE ON)
