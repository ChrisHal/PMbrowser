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
 	DisplayTrace.cpp DisplayTrace.h
    DlgChoosePathAndPrefix.cpp DlgChoosePathAndPrefix.h 
    DlgGraphSettings.cpp DlgGraphSettings.h 
    DlgSelectParameters.cpp DlgSelectParameters.h 
    DlgTreeFilter.cpp DlgTreeFilter.h
    DlgExportMetadata.h DlgExportMetadata.cpp
    main.cpp
    pmbrowserwindow.cpp pmbrowserwindow.h
    renderarea.cpp renderarea.h 

	QtPMbrowser.qrc
	QtPMbrowser.rc

    DlgChoosePathAndPrefix.ui
	DlgGraphSettings.ui
	DlgSelectParameters.ui
	DlgTreeFilter.ui
    DlgExportMetadata.ui
	pmbrowserwindow.ui
	renderarea.ui
)

target_link_libraries(QtPMbrowser PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
target_link_libraries(QtPMbrowser PUBLIC hekatoolslib)

configure_file(Config.h.in Config.h)
configure_file(QtPMbrowser.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/QtPMbrowser.rc)
target_include_directories(QtPMbrowser PUBLIC "${PROJECT_BINARY_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_definitions(QtPMbrowser PRIVATE
        NDEBUG
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.

# deploy script
if(WIN32)
get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
set(ms_deploy_dir "${CMAKE_CURRENT_BINARY_DIR}/deploy")
message(STATUS "deploying to " ${ms_deploy_dir})
add_custom_target(deploy ALL
#    COMMAND powershell -Command "if ((Test-Path ${ms_deploy_dir})) {rm -r ${ms_deploy_dir}}"
	COMMAND ${WINDEPLOYQT_EXECUTABLE} --no-compiler-runtime --dir ${ms_deploy_dir}         \"$<TARGET_FILE:QtPMbrowser>\"
    COMMENT "Deploying Qt..."
)
add_dependencies(deploy QtPMbrowser)
install(TARGETS QtPMbrowser RUNTIME)
install(DIRECTORY ${ms_deploy_dir}/ TYPE BIN)
elseif(APPLE)

install(TARGETS QtPMbrowser BUNDLE DESTINATION ./ COMPONENT pmbrowser)
cpack_add_component(pmbrowser)

get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
add_custom_command(TARGET QtPMbrowser POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env "${MACDEPLOYQT_EXECUTABLE}"
        \"$<TARGET_BUNDLE_DIR:QtPMbrowser>\"
    COMMENT "Deploying Qt..."
)

else()
install(TARGETS QtPMbrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endif()

