project(QtPMbrowser LANGUAGES CXX)

add_library(hekatoolslib STATIC
           "DatFile.cpp" "DatFile.h"
           "exportIBW.cpp" "exportIBW.h"
           "helpers.cpp" "helpers.h"
           "Igor_IBW.h"
           "igor_ipf.h"
           "hkTree.cpp" "hkTree.h"
           "time_handling.cpp" "time_handling.h"
           "PMparameters.cpp" "PMparameters.h"
           "machineinfo.h"
           "StimTree.h" "StimTree.cpp" "exportNPY.cpp" "exportNPY.h"
           "hkTreeView.h" "hkTreeView.cpp")

target_include_directories(hekatoolslib
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_definitions(hekatoolslib PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
