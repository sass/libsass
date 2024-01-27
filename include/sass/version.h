/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VERSION_H
#define SASS_VERSION_H
  
// In order to report something useful this constant must be
// passed during compilation (e.g. -DLIBSASS_VERSION=\"x.y.z\").
// Note: what you pass must be a valid string with quotes!
#ifndef LIBSASS_VERSION
#define LIBSASS_VERSION "[NA]"
#endif

#ifndef LIBSASS_LANGUAGE_VERSION
#define LIBSASS_LANGUAGE_VERSION "3.9"
#endif

#endif
