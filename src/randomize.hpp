/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_RANDOMIZE_HPP
#define SASS_RANDOMIZE_HPP

#include <cstdint>

// Macros for env variables
#ifdef _WIN32
#define GET_ENV(key) getenv(key)
#define SET_ENV(key, val) _putenv_s(key, val)
#else
#define GET_ENV(key) getenv(key)
#define SET_ENV(key, val) setenv(key, val, 0)
#endif

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Just a static wrapped around random device
  // Creates one true random number to seed us
  uint32_t getHashSeed(uint32_t* preset = nullptr);

  // Create a random `int` between [low] and [high]
  int getRandomInt(int low = 0.0, int high = 1.0);

  // Create a random `float` between [low] and [high]
  float getRandomFloat(float low = 0.0, float high = 1.0);

  // Create a random `double` between [low] and [high]
  double getRandomDouble(double low = 0.0, double high = 1.0);

  // Get full 32bit random data
  uint32_t getRandomUint32();

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

};

#endif
