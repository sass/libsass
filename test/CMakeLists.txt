cmake_minimum_required( VERSION 2.8 )

project( LibSassTests C )

if( $ENV{LIBSASS_TEST_SPEC_BRANCH} )
  set( SPEC_BRANCH $ENV{LIBSASS_TEST_SPEC_BRANCH} )
else()
  set( SPEC_BRANCH master )
endif()

if( NOT DEFINED $ENV{LIBSASS_TEST_SKIP_DOWNLOAD_DEPS} )
  if(WIN32)
    execute_process( COMMAND powershell "rm -r -fo ${CMAKE_CURRENT_LIST_DIR}/Criterion 2>&1> $null" )
    execute_process( COMMAND powershell "rm -r -fo ${CMAKE_CURRENT_LIST_DIR}/sass-spec 2>&1> $null" )
    execute_process( COMMAND powershell "rm -r -fo ${CMAKE_CURRENT_LIST_DIR}/slre 2>&1> $null" )
    execute_process( COMMAND powershell "rm -r -fo ${CMAKE_CURRENT_LIST_DIR}/tinydir 2>&1> $null" )
  else()
    execute_process( COMMAND rm -rf ${CMAKE_CURRENT_LIST_DIR}/sass-spec ${CMAKE_CURRENT_LIST_DIR}/Criterion ${CMAKE_CURRENT_LIST_DIR}/tinydir ${CMAKE_CURRENT_LIST_DIR}/slre 2>1> /dev/null )
  endif()

  execute_process( COMMAND git clone https://github.com/Snaipe/Criterion --recursive ${CMAKE_CURRENT_LIST_DIR}/Criterion )
  execute_process( COMMAND git clone https://github.com/sass/sass-spec --recursive -b ${SPEC_BRANCH} ${CMAKE_CURRENT_LIST_DIR}/sass-spec )
  execute_process( COMMAND git clone https://github.com/cesanta/slre --recursive ${CMAKE_CURRENT_LIST_DIR}/slre )
  execute_process( COMMAND git clone https://github.com/cxong/tinydir --recursive ${CMAKE_CURRENT_LIST_DIR}/tinydir )
endif()

add_subdirectory( Criterion )

add_executable( RUN ./spec_theories.c Criterion/src/ slre/slre.c )

add_library( libsass STATIC IMPORTED )

if( MSVC )
  set_target_properties( libsass PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../win/bin/libsass.lib )
else()
  set_target_properties( libsass PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../build/lib/libsass.a )
endif()

target_link_libraries( RUN criterion libsass )

include_directories(
  Criterion/include/
  ../include/
)
