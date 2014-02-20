# Michael Aaron Safyan (michaelsafyan@gmail.com). Copyright (C) 2009. Simplified BSD License.

#
# This CMake package creates a Doxygen documentation target.
#

FIND_PACKAGE (Doxygen)
IF (DOXYGEN_FOUND)
    IF (EXISTS ${PROJECT_SOURCE_DIR}/Doxyfile)
        ADD_CUSTOM_TARGET(
            doxygen
            ${DOXYGEN_EXECUTABLE} Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating doxygen project documentation." VERBATIM
        )
        ADD_CUSTOM_TARGET(
            documentation
            ${DOXYGEN_EXECUTABLE} Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating doxygen project documentation." VERBATIM
        )
        ADD_CUSTOM_TARGET(
            docs
            ${DOXYGEN_EXECUTABLE} Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating doxygen project documentation." VERBATIM
        )
    ELSE (EXISTS ${PROJECT_SOURCE_DIR}/Doxyfile)
        ADD_CUSTOM_TARGET(doxygen COMMENT "Doxyfile not found. Please generate a doxygen configuration file to use this target." VERBATIM)
        ADD_CUSTOM_TARGET(documentation COMMENT "Doxyfile not found. Please generate a doxygen configuration file to use this target." VERBATIM)
        ADD_CUSTOM_TARGET(docs COMMENT "Doxyfile not found. Please generate a doxygen configuration file to use this target." VERBATIM)
    ENDIF (EXISTS ${PROJECT_SOURCE_DIR}/Doxyfile)
ELSE (DOXYGEN_FOUND)
    ADD_CUSTOM_TARGET(doxygen COMMENT "Doxygen not found. Please install doxygen to use this target." VERBATIM)
    ADD_CUSTOM_TARGET(documentation COMMENT "Doxygen not found. Please install doxygen to use this target." VERBATIM)
    ADD_CUSTOM_TARGET(docs COMMENT "Doxygen not found. Please install doxygen to use this target." VERBATIM)
ENDIF (DOXYGEN_FOUND)
