# set(CMAKE_GENERATOR "MinGW Makefiles")
set(CMAKE_GENERATOR "Visual Studio 17 2022")
message("generator is set to ${CMAKE_GENERATOR}")

cmake_minimum_required(VERSION 4.0)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows")
endif()
message(STATUS "VCPKG_TARGET_TRIPLET is ${VCPKG_TARGET_TRIPLET}")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")
    else()
        message(FATAL_ERROR "VCPKG_ROOT environment variable is not set")
    endif()
endif()
message(STATUS "CMAKE_TOOLCHAIN_FILE is ${CMAKE_TOOLCHAIN_FILE}")

include("${CMAKE_TOOLCHAIN_FILE}")

project(ifdcm)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# MSVC
add_compile_options(/Zc:__cplusplus)

# DCMTK の検索
find_package(DCMTK REQUIRED)
set(DCMTK_INSTALL_DIR "C:/dev/vcpkg/installed/${VCPKG_TARGET_TRIPLET}")
set(DCMTK_LIBRARY_DIRS ${DCMTK_INSTALL_DIR}/lib)
message(STATUS "DCMTK_INCLUDE_DIRS: ${DCMTK_INCLUDE_DIRS}")
message(STATUS "DCMTK_LIBRARY_DIRS: ${DCMTK_LIBRARY_DIRS}")

link_directories(${DCMTK_LIBRARY_DIRS})

file(GLOB_RECURSE cppfiles RELATIVE ${PROJECT_SOURCE_DIR} ./src/*.cpp)
add_library(ifdcm SHARED ${cppfiles})

target_include_directories(ifdcm PRIVATE
    ${DCMTK_INCLUDE_DIRS}
    ./include
)

# DCMTK のリンク設定
target_link_libraries(ifdcm PRIVATE
    dcmdata
    dcmimgle
    dcmimage
    ofstd
)

set_target_properties(ifdcm PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ifdcm"
    SUFFIX ".sph"
)
