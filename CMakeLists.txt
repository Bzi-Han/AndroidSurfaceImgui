cmake_minimum_required(VERSION 3.13.0)

project(AndroidSurfaceImgui)

option(ANDROID_SURFACE_IMGUI_BUILD_STATIC "Build AImGui static library." ON)
option(ANDROID_SURFACE_IMGUI_BUILD_SHARED "Build test library." ON)
option(ANDROID_SURFACE_IMGUI_BUILD_TESTING "Build test programs." ON)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fno-rtti -fvisibility=hidden)
add_link_options(-s)

find_library(liblog log NO_CACHE)

include_directories(
    modules
    includes
    third_party
    third_party/imgui
    third_party/zstd/lib
)

# Scan imgui sources
aux_source_directory(third_party/imgui AIMGUI_IMGUI_SOURCES)
# Make imgui backends sources
set(
    AIMGUI_IMGUI_BACKENDS_SOURCES
    third_party/imgui/backends/imgui_impl_android.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# Scan common sources
aux_source_directory(src/common AIMGUI_COMMON_SOURCES)

# Build zstd library
set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_SHARED OFF)
add_subdirectory(third_party/zstd/build/cmake)

# Build AImGui library
if(ANDROID_SURFACE_IMGUI_BUILD_STATIC)
    add_library(AImGui STATIC ${AIMGUI_COMMON_SOURCES} ${AIMGUI_IMGUI_SOURCES} ${AIMGUI_IMGUI_BACKENDS_SOURCES})
    target_link_libraries(AImGui ${liblog} EGL GLESv3 android libzstd_static)
    set_target_properties(AImGui PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "libs/${ANDROID_ABI}"
    )
endif()

# Build test library
if(ANDROID_SURFACE_IMGUI_BUILD_SHARED)
    add_library(test SHARED src/core/main.cc ${AIMGUI_COMMON_SOURCES} ${AIMGUI_IMGUI_SOURCES} ${AIMGUI_IMGUI_BACKENDS_SOURCES})
    target_link_libraries(test ${liblog} EGL GLESv3 android libzstd_static)
    set_target_properties(test PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "libs/${ANDROID_ABI}"
    )
endif()

# Build test-ui programs
if(ANDROID_SURFACE_IMGUI_BUILD_TESTING)
    add_executable(test-ui src/test-ui/main.cc ${AIMGUI_COMMON_SOURCES} ${AIMGUI_IMGUI_SOURCES} ${AIMGUI_IMGUI_BACKENDS_SOURCES})
    target_link_libraries(test-ui ${liblog} EGL GLESv3 android libzstd_static)
    set_target_properties(test-ui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
    )
    add_executable(canvas-ui src/test-ui/canvas.cc ${AIMGUI_COMMON_SOURCES} ${AIMGUI_IMGUI_SOURCES} ${AIMGUI_IMGUI_BACKENDS_SOURCES})
    target_link_libraries(canvas-ui ${liblog} EGL GLESv3 android libzstd_static)
    set_target_properties(canvas-ui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
    )
    add_executable(render-ui src/test-ui/render.cc ${AIMGUI_COMMON_SOURCES} ${AIMGUI_IMGUI_SOURCES} ${AIMGUI_IMGUI_BACKENDS_SOURCES})
    target_link_libraries(render-ui ${liblog} EGL GLESv3 android libzstd_static)
    set_target_properties(render-ui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
    )
endif()