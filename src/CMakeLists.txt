# Hampton Catlin. Copyright (C) 2012. MIT

SET (project_SRCS
	ast.cpp
	base64vlq.cpp
	bind.cpp
	cencode.c
	color_maps.cpp
	constants.cpp
	context.cpp
	cssize.cpp
	emitter.cpp
	environment.cpp
	error_handling.cpp
	eval.cpp
	expand.cpp
	extend.cpp
	file.cpp
	functions.cpp
	inspect.cpp
	json.cpp
	lexer.cpp
	listize.cpp
	memory_manager.cpp
	node.cpp
	output.cpp
	parser.cpp
	plugins.cpp
	position.cpp
	prelexer.cpp
	remove_placeholders.cpp
	sass.cpp
	sass_util.cpp
	sass_values.cpp
	sass_context.cpp
	sass_functions.cpp
	sass_interface.cpp
	sass2scss.cpp
	source_map.cpp
	to_c.cpp
	to_string.cpp
	to_value.cpp
	units.cpp
	utf8_string.cpp
	values.cpp
	util.cpp
)
SET (project_LIBS )
SET (project_BIN ${PROJECT_NAME})

IF (NOT DEFINED project_build_shared)
	IF (CMAKE_CROSSCOMPILING)
		SET(project_build_shared 0)
	ELSE (CMAKE_CROSSCOMPILING)
		SET(project_build_shared 1)
	ENDIF (CMAKE_CROSSCOMPILING)
ENDIF (NOT DEFINED project_build_shared)

IF (NOT DEFINED project_build_static)
	SET(project_build_static 1)
ENDIF (NOT DEFINED project_build_static)

IF (project_build_static)
	ADD_LIBRARY(staticlib STATIC ${project_SRCS})
	TARGET_LINK_LIBRARIES(staticlib ${project_LIBS})
	SET_TARGET_PROPERTIES(staticlib PROPERTIES VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}" OUTPUT_NAME ${project_BIN} CLEAN_DIRECT_OUTPUT 1)
	INSTALL(TARGETS staticlib DESTINATION lib)
ENDIF(project_build_static)

IF (project_build_shared)
	ADD_LIBRARY(sharedlib MODULE ${project_SRCS})
	TARGET_LINK_LIBRARIES(sharedlib ${project_LIBS})
	SET_TARGET_PROPERTIES(sharedlib PROPERTIES VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}" OUTPUT_NAME ${project_BIN} CLEAN_DIRECT_OUTPUT 1)
	INSTALL(TARGETS sharedlib DESTINATION lib)
ENDIF(project_build_shared)
