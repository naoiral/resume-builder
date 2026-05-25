cmake_minimum_required(VERSION 3.20)
project(ResumeBuilder LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
include(cmake/platform.cmake)

# Resource embedding helper
include(cmake/embed_resource.cmake)

# Third party include dirs
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ========================================================================
# zlib (for libharu)
# ========================================================================
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/zlib EXCLUDE_FROM_ALL)

# ========================================================================
# libharu
# ========================================================================
set(LIBHPDF_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Point libharu to our zlib
set(ZLIB_FOUND TRUE)
set(ZLIB_INCLUDE_DIRS $<TARGET_PROPERTY:zlibstatic,INCLUDE_DIRECTORIES>)
set(ZLIB_LIBRARIES zlibstatic)

# Disable PNG (we don't need images in PDF)
set(PNG_FOUND FALSE)

add_subdirectory(${THIRD_PARTY_DIR}/libharu EXCLUDE_FROM_ALL)

# Fix libharu include paths
target_include_directories(hpdf PRIVATE
    ${THIRD_PARTY_DIR}/libharu/include
    $<TARGET_PROPERTY:zlibstatic,INCLUDE_DIRECTORIES>
)

# Force static CRT for all third party libraries
if(MSVC)
    set_property(TARGET hpdf PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    set_property(TARGET zlibstatic PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# ========================================================================
# ImGui sources
# ========================================================================
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

if(PLATFORM_DESKTOP)
    if(PLATFORM_WINDOWS)
        list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_win32.cpp)
    elseif(PLATFORM_MACOS)
        list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_osx.mm)
    endif()
elseif(PLATFORM_ANDROID)
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_android.cpp)
elseif(PLATFORM_IOS)
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_osx.mm)
endif()

# ========================================================================
# Generate font data (use a placeholder if subset font not yet built)
# ========================================================================
set(FONT_SUBSET_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/NotoSansSC-Regular-subset.ttf")
if(NOT EXISTS "${FONT_SUBSET_FILE}")
    # Create a minimal placeholder font file (1KB) for initial build
    set(FONT_SUBSET_FILE "${CMAKE_CURRENT_BINARY_DIR}/placeholder_font.bin")
    file(WRITE "${FONT_SUBSET_FILE}" "PLACEHOLDER")
endif()

embed_resource(FONT_DATA "${FONT_SUBSET_FILE}")

# ========================================================================
# Application sources
# ========================================================================
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/ui/ui_main.cpp
    src/ui/ui_editor.cpp
    src/ui/ui_preview.cpp
    src/ui/ui_theme.cpp
    src/ui/modules/mod_personal.cpp
    src/ui/modules/mod_education.cpp
    src/ui/modules/mod_work.cpp
    src/ui/modules/mod_projects.cpp
    src/ui/modules/mod_skills.cpp
    src/ui/modules/mod_certs.cpp
    src/data/resume_model.cpp
    src/data/json_io.cpp
    src/data/autosave.cpp
    src/template/template_engine.cpp
    src/template/tpl_classic.cpp
    src/template/tpl_modern.cpp
    src/template/tpl_minimal.cpp
    src/template/tpl_creative.cpp
    src/export/pdf_export.cpp
    ${FONT_DATA_OUTPUT}
)

# ========================================================================
# Platform-specific entry points
# ========================================================================
if(PLATFORM_ANDROID)
    list(APPEND APP_SOURCES src/platform/platform_android.cpp)
elseif(PLATFORM_IOS)
    list(APPEND APP_SOURCES src/platform/platform_ios.cpp)
else()
    list(APPEND APP_SOURCES src/platform/platform_desktop.cpp)
endif()

# ========================================================================
# Build target
# ========================================================================
if(PLATFORM_ANDROID OR PLATFORM_IOS)
    add_library(resume_builder SHARED ${APP_SOURCES} ${IMGUI_SOURCES})
else()
    add_executable(resume_builder ${APP_SOURCES} ${IMGUI_SOURCES})
endif()

target_include_directories(resume_builder PRIVATE
    ${THIRD_PARTY_DIR}
    ${THIRD_PARTY_DIR}/libharu/include
    ${CMAKE_CURRENT_BINARY_DIR}/third_party/libharu/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(resume_builder PRIVATE hpdf zlibstatic)

# ========================================================================
# Platform-specific linking
# ========================================================================
if(PLATFORM_WINDOWS)
    target_link_libraries(resume_builder PRIVATE opengl32 dwmapi)
    target_compile_definitions(resume_builder PRIVATE
        _CRT_SECURE_NO_WARNINGS
        IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    )
elseif(PLATFORM_MACOS)
    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    target_link_libraries(resume_builder PRIVATE
        ${OPENGL_FRAMEWORK}
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
    )
    set_target_properties(resume_builder PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.resumebuilder.app"
        MACOSX_BUNDLE_BUNDLE_NAME "ResumeBuilder"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
elseif(PLATFORM_LINUX)
    target_link_libraries(resume_builder PRIVATE GL X11 Xext Xi dl pthread)
elseif(PLATFORM_ANDROID)
    target_link_libraries(resume_builder PRIVATE android EGL GLESv3 log)
elseif(PLATFORM_IOS)
    find_library(METAL Metal)
    find_library(UIKIT UIKit)
    find_library(COREGRAPHICS CoreGraphics)
    find_library(QUARTZCORE QuartzCore)
    target_link_libraries(resume_builder PRIVATE
        ${METAL} ${UIKIT} ${COREGRAPHICS} ${QUARTZCORE}
        "-framework Foundation"
    )
endif()

# ========================================================================
# Compile definitions
# ========================================================================
target_compile_definitions(resume_builder PRIVATE
    APP_VERSION="1.0.0"
    PLATFORM_NAME="${PLATFORM_NAME}"
)

if(PLATFORM_DESKTOP)
    target_compile_definitions(resume_builder PRIVATE PLATFORM_DESKTOP=1)
elseif(PLATFORM_MOBILE)
    target_compile_definitions(resume_builder PRIVATE PLATFORM_MOBILE=1)
endif()

# ========================================================================
# MSVC specific
# ========================================================================
if(MSVC)
    target_compile_options(resume_builder PRIVATE /W3 /D_CRT_SECURE_NO_WARNINGS)
    set_property(TARGET resume_builder PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded"
    )
    # Use WinMain entry point for Windows GUI app
    set_target_properties(resume_builder PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# ========================================================================
# Copy fonts directory to build output
# ========================================================================
add_custom_command(TARGET resume_builder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        $<TARGET_FILE_DIR:resume_builder>/fonts
    COMMENT "Copying fonts to output directory"
)

# ========================================================================
# PDF font test (temporary)
# ========================================================================
add_executable(test_pdf_direct test_pdf_direct.cpp)
target_include_directories(test_pdf_direct PRIVATE
    ${THIRD_PARTY_DIR}/libharu/include
    ${CMAKE_CURRENT_BINARY_DIR}/third_party/libharu/include
)
target_link_libraries(test_pdf_direct PRIVATE hpdf zlibstatic)
if(MSVC)
    set_property(TARGET test_pdf_direct PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
