add_library(hekatoolslib STATIC
           DatFile.cpp DatFile.h
           exportIBW.cpp exportIBW.h
           helpers.cpp helpers.h
           Igor_IBW.h
           igor_ipf.h
           hkTree.cpp hkTree.h
           time_handling.cpp time_handling.h
           PMparameters.cpp PMparameters.h
           machineinfo.h
)

target_include_directories(hekatoolslib
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
