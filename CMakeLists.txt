# ============================================================================
#  Weighted Voronoi Stippling
# ----------------------------------------------------------------------------
#  A CMake project. CMake generates the build files for whatever toolchain is
#  present (here: MSVC / Visual Studio) and MSVC does the actual compiling.
#
#  This file describes the project abstractly, so it contains no compiler
#  commands and no machine-specific absolute paths. It builds unchanged on any
#  machine that has CMake + the MSVC C++ toolset.
# ============================================================================
cmake_minimum_required(VERSION 3.21)

project(stippling
    VERSION 0.1.0
    DESCRIPTION "Weighted Voronoi Stippling"
    LANGUAGES CXX)

# ---- Language standard ------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Sources ----------------------------------------------------------------
# CONFIGURE_DEPENDS makes CMake re-scan for new/removed files on each build, so
# you can just drop .cpp/.h files into src/ without editing this file.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")

add_executable(stippling ${SOURCES})
target_include_directories(stippling PRIVATE src)

# Header-only third-party libraries (stb_image, stb_image_write) live in lib/.
# SYSTEM marks them as system headers so their warnings stay out of your /W4 build.
target_include_directories(stippling SYSTEM PRIVATE lib)

# ---- Compiler options -------------------------------------------------------
if(MSVC)
    # /W4 high warnings, /EHsc standard C++ exceptions.
    target_compile_options(stippling PRIVATE /W4 /EHsc)
endif()
