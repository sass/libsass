# Hampton Catlin. Copyright (C) 2012. MIT

FILE (GLOB_RECURSE project_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
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
	ADD_LIBRARY(sharedlib SHARED ${project_SRCS})
	TARGET_LINK_LIBRARIES(sharedlib ${project_LIBS})
	SET_TARGET_PROPERTIES(sharedlib PROPERTIES VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}" OUTPUT_NAME ${project_BIN} CLEAN_DIRECT_OUTPUT 1)
	INSTALL(TARGETS sharedlib DESTINATION lib)
ENDIF(project_build_shared)
