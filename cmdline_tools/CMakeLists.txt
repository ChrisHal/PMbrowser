project(QtPMbrowser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(metadataextractor "metadataextractor.cpp")
target_link_libraries(metadataextractor PUBLIC hekatoolslib)
add_executable(stimtree_explorer "stimtree_explorer.cpp")
target_link_libraries(stimtree_explorer PUBLIC hekatoolslib)

