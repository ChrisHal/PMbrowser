project(QtPMbrowser)
find_package(Sphinx REQUIRED)

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/html)

configure_file(	"${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
				"${CMAKE_CURRENT_BINARY_DIR}/conf.py"
				@ONLY)


add_custom_target(Sphinx ALL
                  COMMAND
                  ${SPHINX_EXECUTABLE} -c ${CMAKE_CURRENT_BINARY_DIR} -b html
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Generating documentation with Sphinx")
				  
install(DIRECTORY ${SPHINX_BUILD} TYPE DOC)
		  