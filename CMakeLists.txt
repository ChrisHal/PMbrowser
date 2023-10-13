cmake_minimum_required(VERSION 3.16)
project(QtPMbrowser VERSION 2.4.0 LANGUAGES CXX)
set(MY_COPYRIGHT "2020 - 2023 Christian R. Halaszovich")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_DOCS "build documentation" OFF)

include(GNUInstallDirs)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "PMbrowser")
set(CPACK_RESOURCE_FILE_LICENSE  ${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt)
set(CPACK_PACKAGE_CONTACT "christian@halaszovich.de")
set(CPACK_PACKAGE_DESCRIPTION "tool to browse HEKA dat files and export to IgorPro")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tool to browse HEKA dat files and export to IgorPro")
set(CPACK_PACKAGE_VENDOR "CRHalaszovichMD")
set(CPACK_DEBIAN_PACKAGE_SECTION "science/utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6gui6,libqt6core6,libqt6widgets6,qt6-qpa-plugins,libxkbcommon0")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://www.halaszovich.de/pmbrowser/html/index.html")
set(CPACK_WIX_UPGRADE_GUID FED5BE2C-2537-49A8-B5D8-39C00D289CC6)
set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/QtPMbrowser/myappico.ico)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/QtPMbrowser/myappico.ico)
set(CPACK_WIX_PROGRAM_MENU_FOLDER "PM browser")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PM browser")
set(CPACK_PACKAGE_EXECUTABLES "QtPMbrowser;PM browser")
include(CPack)


if (MSVC)
    # warning level 4 and some
    add_compile_options(/W4 /permissive- /sdl /EHsc)
else()
    # lots of warnings
    add_compile_options(-Wall -Wextra -pedantic 
	#-Werror
	)
endif()

add_subdirectory("hekatoolslib")
add_subdirectory("cmdline_tools")
add_subdirectory("QtPMbrowser")
if(BUILD_DOCS)
add_subdirectory("doc")
endif()
