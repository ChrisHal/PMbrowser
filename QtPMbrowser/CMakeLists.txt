cmake_minimum_required(VERSION 3.16)
project(QtPMbrowser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set (CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.0.1\\5.0.1\\msvc2010\\")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
#include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

add_executable(QtPMbrowser WIN32 MACOSX_BUNDLE
    DatFile.cpp DatFile.h
    DisplayTrace.cpp DisplayTrace.h
    DlgChoosePathAndPrefix.cpp DlgChoosePathAndPrefix.h DlgChoosePathAndPrefix.ui
    DlgGraphSettings.cpp DlgGraphSettings.h DlgGraphSettings.ui
    DlgSelectParameters.cpp DlgSelectParameters.h DlgSelectParameters.ui
    DlgTreeFilter.cpp DlgTreeFilter.h DlgTreeFilter.ui
    Igor_IBW.h
    PMparameters.cpp PMparameters.h
    exportIBW.cpp exportIBW.h
    helpers.cpp helpers.h
    hkTree.cpp hkTree.h
    machineinfo.h
    main.cpp
    pmbrowserwindow.cpp pmbrowserwindow.h pmbrowserwindow.ui
    renderarea.cpp renderarea.h renderarea.ui
    time_handling.cpp time_handling.h
	QtPMbrowser.qrc
	QtPMbrowser.rc
)


target_link_libraries(QtPMbrowser PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

configure_file(Config.h.in Config.h)
target_include_directories(QtPMbrowser PUBLIC "${PROJECT_BINARY_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_definitions(QtPMbrowser PRIVATE
        NDEBUG
    )
endif()

install(TARGETS QtPMbrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.

# deploy script for Qt5
if(WIN32)
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
add_custom_command(TARGET QtPMbrowser POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime # will be added by CPack
            \"$<TARGET_FILE:QtPMbrowser>\"
    COMMENT "Deploying Qt..."
)
endif()

