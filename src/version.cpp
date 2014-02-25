#include <sass/version.h>
#include "libraryinfo.h"

extern "C" {

int sass_version_major()
{
  return LIBRARY_VERSION_MAJOR;
}

int sass_version_minor()
{
  return LIBRARY_VERSION_MINOR;
}

int sass_version_patch()
{
  return LIBRARY_VERSION_PATCH;
}

}
