cmake_minimum_required( VERSION 3.10 )
project(find-identical-img)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF )

## export version number in version file
set (FII_NAME "fii")
set (FII_FULLNAME "Find Identical Images")
set (FII_VERSION_MAJOR "0")
set (FII_VERSION_MINOR "0")
set (FII_VERSION_PATCH "1")
configure_file (
  "${PROJECT_SOURCE_DIR}/fii_version.h.in"
  "${PROJECT_SOURCE_DIR}/fii_version.h"
  )

IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
ENDIF()

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
find_library(CMAKE_RT_LIB rt)
find_library(CMAKE_M_LIB m)

add_library(fii_util fii_util.cc)

add_executable(fii fii.cc)
target_link_libraries(
 fii fii_util
 Threads::Threads OpenMP::OpenMP_CXX
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
#add_executable(fii_inspect_img fii_inspect_img.cc)

## tests
ENABLE_TESTING()
add_executable(fii_image_size_test fii_image_size_test.cc)
target_link_libraries(
 fii_image_size_test fii_util
 Threads::Threads OpenMP::OpenMP_CXX
)

add_executable(fii_test fii_test.cc)
target_link_libraries(
 fii_test fii_util
 Threads::Threads OpenMP::OpenMP_CXX
)

add_test(fii_image_size fii_image_size_test)
add_test(fii fii_test)
