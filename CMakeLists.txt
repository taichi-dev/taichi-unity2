cmake_minimum_required(VERSION 3.15)

project(taichi_unity LANGUAGES C CXX OBJC OBJCXX)

option(TAICHI_UNITY_BUILD_STATIC "Build taichi_unity as static instaed of shared library")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE PATH "Build Type" FORCE)
endif()

if (NOT EXISTS ${TAICHI_C_API_INSTALL_DIR})
    set(TAICHI_C_API_INSTALL_DIR $ENV{TAICHI_C_API_INSTALL_DIR})
endif()

message("-- Taichi Runtime C-API: ${TAICHI_C_API_INSTALL_DIR}")

# https://discourse.cmake.org/t/cmake-does-not-find-libraries-when-compiling-for-android/5098
if (ANDROID)
    list(APPEND CMAKE_FIND_ROOT_PATH ${TAICHI_C_API_INSTALL_DIR}/lib)
endif()

find_library(taichi_c_api taichi_c_api HINTS ${TAICHI_C_API_INSTALL_DIR}/lib REQUIRED NO_CMAKE_FIND_ROOT_PATH)

set(INC_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/Vulkan-Headers/include
    ${TAICHI_C_API_INSTALL_DIR}/include
)
set(LINK_LIBS ${taichi_c_api})

include_directories(${INC_DIRS})
file(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/*")
file(GLOB INCS "${PROJECT_SOURCE_DIR}/include/*")
if (TAICHI_UNITY_BUILD_STATIC)
    add_library(taichi_unity STATIC ${SRCS} ${INCS})
else()
    add_library(taichi_unity SHARED ${SRCS} ${INCS})
endif()
target_link_libraries(taichi_unity ${LINK_LIBS})

if (APPLE)
    target_compile_definitions(taichi_unity PUBLIC -DTI_WITH_METAL=1)
else()
    target_compile_definitions(taichi_unity PUBLIC -DTI_WITH_VULKAN=1)
endif()

install(TARGETS taichi_unity EXPORT taichi_unityTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
