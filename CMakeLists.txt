cmake_minimum_required(VERSION 3.15)

project(taichi_unity LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE PATH "Build Type" FORCE)
endif()

find_package(Vulkan REQUIRED)

set(TAICHI_C_API_INSTALL_DIR $ENV{TAICHI_REPO_DIR}/_skbuild/win-amd64-3.8/cmake-install/c_api CACHE PATH "path to taichi c-api installation directory")

find_library(taichi_c_api taichi_c_api HINTS ${TAICHI_C_API_INSTALL_DIR}/lib REQUIRED)

set(INC_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    ${TAICHI_C_API_INSTALL_DIR}/include
)
set(LINK_LIBS
    ${taichi_c_api}
    ${Vulkan_LIBRARY}
)

include_directories(${INC_DIRS})
file(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/*")
file(GLOB INCS "${PROJECT_SOURCE_DIR}/include/*")
add_library(taichi_unity SHARED ${SRCS} ${INCS})
target_link_libraries(taichi_unity ${LINK_LIBS})
