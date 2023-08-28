cmake_minimum_required(VERSION 3.0.0)

project(AndroidSurfaceImgui)

set(CMAKE_CXX_STANDARD 20)

find_library(liblog log)

include_directories(
    modules
    includes
    third_party
    third_party/imgui
)

# Scan imgui sources
aux_source_directory(third_party/imgui IMGUI_SOURCES)
# Make imgui backends sources
set(
    IMGUI_BACKENDS_SOURCES
    third_party/imgui/backends/imgui_impl_android.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# Scan common sources
aux_source_directory(src/common COMMON_SOURCES)

# Build test library
add_library(test SHARED src/core/main.cc ${COMMON_SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})
target_link_libraries(test ${liblog} EGL GLESv3 android)
set_target_properties(test PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "libs/${ANDROID_ABI}"
)

# Build test-ui programs
add_executable(test-ui src/test-ui/main.cc ${COMMON_SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})
target_link_libraries(test-ui ${liblog} EGL GLESv3 android)
set_target_properties(test-ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
)
add_executable(canvas-ui src/test-ui/canvas.cc ${COMMON_SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})
target_link_libraries(canvas-ui ${liblog} EGL GLESv3 android)
set_target_properties(canvas-ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
)
add_executable(render-ui src/test-ui/render.cc ${COMMON_SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})
target_link_libraries(render-ui ${liblog} EGL GLESv3 android)
set_target_properties(render-ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "tests/${ANDROID_ABI}"
)