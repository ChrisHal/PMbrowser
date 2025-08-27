project(QtPMbrowser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(MACOSX_BUNDLE_BUNDLE_NAME "PM browser")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}")
set(MACOSX_BUNDLE_COPYRIGHT "${MY_COPYRIGHT}")
set(MACOSX_BUNDLE_ICON_FILE PMbrowser.icns)

# qt_standard_project_setup() does not set CMAKE_AUTORCC
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup(REQUIRES 6.7)

set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/PMbrowser.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")

qt_add_executable(QtPMbrowser WIN32 MACOSX_BUNDLE
 	DisplayTrace.cpp DisplayTrace.h
    DlgChoosePathAndPrefix.cpp DlgChoosePathAndPrefix.h 
    DlgGraphSettings.cpp DlgGraphSettings.h 
    DlgSelectParameters.cpp DlgSelectParameters.h 
    DlgTreeFilter.cpp DlgTreeFilter.h
    DlgExportMetadata.h DlgExportMetadata.cpp
    main.cpp
    pmbrowserwindow.cpp pmbrowserwindow.h
	"PMparametersModel.cpp" "PMparametersModel.h"
    renderarea.cpp renderarea.h 

    DlgPreferences.cpp
    DlgPreferences.h
    DlgPreferences.ui

	QtPMbrowser.qrc
	${PROJECT_BINARY_DIR}/QtPMbrowser.rc
	${app_icon_macos}

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
configure_file(QtPMbrowser.rc.in ${PROJECT_BINARY_DIR}/QtPMbrowser.rc)
target_include_directories(QtPMbrowser PUBLIC "${PROJECT_BINARY_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_definitions(QtPMbrowser PRIVATE
        NDEBUG
    )
endif()

# deploy script
if(WIN32 OR APPLE)
	install(TARGETS QtPMbrowser
	RUNTIME
	BUNDLE DESTINATION .)
	qt_generate_deploy_app_script(
    TARGET QtPMbrowser
    OUTPUT_SCRIPT deploy_script
	NO_COMPILER_RUNTIME
	)
	install(SCRIPT ${deploy_script})
else()
	install(TARGETS QtPMbrowser
		BUNDLE DESTINATION .
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
# install dektop file and icons
	set(MY_APP_ID "de.halaszovich.PMbrowser")
	string(TIMESTAMP my_current_day "%Y-%m-%d")
	configure_file(linux_deploy/de.halaszovich.pmbrowser.desktop.in ${MY_APP_ID}.desktop)
	configure_file(linux_deploy/${MY_APP_ID}.appdata.xml.in ${MY_APP_ID}.appdata.xml)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MY_APP_ID}.desktop" DESTINATION
	"${CMAKE_INSTALL_DATADIR}/applications")
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MY_APP_ID}.appdata.xml" DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo" RENAME "${MY_APP_ID}.metainfo.xml")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/linux_deploy/halaszovich-pmbrowser256.png"  DESTINATION
	"${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps" RENAME ${MY_APP_ID}.png)
endif()


