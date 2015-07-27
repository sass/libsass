# Hampton Catlin. Copyright (C) 2012. MIT

include(CTest)

FILE (GLOB test_SRCS *.cpp)
SET (test_LIBS ${PROJECT_NAME})

LINK_DIRECTORIES(${MAINFOLDER}/lib)

FOREACH(test_SRC ${test_SRCS})
	SET (test_NAME ${test_SRC} )
	string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/test_" "" test_NAME "${test_NAME}")
	string(REGEX REPLACE ".cpp" "" test_NAME "${test_NAME}")
	SET (test_BIN ${test_NAME}-unittests)
	ADD_EXECUTABLE(${test_BIN} ${test_SRC})
	TARGET_LINK_LIBRARIES(${test_BIN} ${test_LIBS})
	ADD_TEST(${test_NAME} ${EXECUTABLE_OUTPUT_PATH}/${test_BIN})
ENDFOREACH()

FIND_PACKAGE (Ruby COMPONENTS "RUBY_EXECUTABLE")
IF (RUBY_EXECUTABLE)
	#Compile SassC for sass-spec run
	SET (SASS_SPEC_PATH ${MAINFOLDER}/sass-spec)
	SET (SASSC_BIN sassc)
	SET (SASSC_BIN_PATH ${EXECUTABLE_OUTPUT_PATH}/${SASSC_BIN})
	SET (SASSC_SRCS ${MAINFOLDER}/sassc/sassc.c)
	include_directories(../src)
	ADD_EXECUTABLE(${SASSC_BIN} ${SASSC_SRCS})
	TARGET_LINK_LIBRARIES(${SASSC_BIN} ${test_LIBS})

	#Run sass-spec tests
	ADD_TEST(sass-spec ${RUBY_EXECUTABLE} ${SASS_SPEC_PATH}/sass-spec.rb -c ${SASSC_BIN_PATH} -s --ignore-todo ${LOG_FLAGS} ${SASS_SPEC_PATH})
ELSE (RUBY_EXECUTABLE)
	message(WARNING "The full sass-spec suite is disabled without the Ruby interpreter.")
ENDIF (RUBY_EXECUTABLE)
