cmake_minimum_required(VERSION 3.16)
project(QtPMbrowser VERSION 2.1 LANGUAGES CXX)
set(MY_COPYRIGHT "2020 - 2022 Christian R. Halaszovich")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_DOCS "build documentation" OFF)

include(GNUInstallDirs)

add_subdirectory("QtPMbrowser")
if(BUILD_DOCS)
add_subdirectory("doc")
endif()
