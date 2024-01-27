/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_sass.hpp"

#include <thread>
#ifdef USE_WIN_CRYPT
#include <windows.h>
#include <wincrypt.h>
#else
#include <random>
#include <ctime>
#endif

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Read a truly random seed
  // This is probably expensive
  uint32_t readHashSeed()
  {
    // Our hash seed
    uint32_t seed = 0;
    // Load optional fixed seed from environment
    // Mainly used to pass the seed to plugins
    if (const char* envseed = GET_ENV("SASS_HASH_SEED")) {
      seed = atol(envseed);
    }
    else {
      #ifdef SassStaticHashSeed
      seed = SassStaticHashSeed;
      #elsifdef USE_WIN_CRYPT
      // Get seed via MS API
      HCRYPTPROV hp = 0;
      CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
      CryptGenRandom(hp, sizeof(seed), (BYTE*)&seed);
      CryptReleaseContext(hp, 0);
      #else
      // Try to get random number from system
      try {
        // Get seed via C++11 API
        std::random_device rd;
        seed = rd();
      }
      // On certain system this can throw since either
      // underlying hardware or software can be buggy.
      // https://github.com/sass/libsass/issues/3151
      catch (std::exception&) {
      }
      // Don't trust anyone to be random, so we
      // add a little entropy of our own.
      seed ^= std::time(NULL) ^ std::clock() ^
        std::hash<std::thread::id>()
        (std::this_thread::get_id());
      // Return entropy
      #endif
    }
    // Use some sensible default
    if (seed == 0) {
      // Fibonacci/Golden Ratio Hashing
      seed = 0x9e3779b9;
    }
    // Return the seed
    return seed;
  }
  // EO readHashSeed

  // Just a static wrapped around random device
  // Creates one true random number to seed us
  uint32_t getHashSeed(uint32_t* preset)
  {
    #ifdef SassStaticHashSeed
    return SassStaticHashSeed
    #else
    // This should be thread safe
    static uint32_t seed = preset
      ? *preset : readHashSeed();
    if (preset) seed = *preset;
    return seed; // thread static
    #endif
  }
  // EO getHashSeed

  // Random number generator only needed in eval phase
  // This makes it safe to reset the hash seed before
  std::mt19937& getRng()
  {
    // Lets hope this is indeed thread safe (seed once)
    static std::mt19937 rng(getHashSeed());
    return rng; // static twister per thread
  }
  // EO getRng

  // Create a random `int` between [low] and [high]
  int getRandomInt(int low, int high)
  {
    static std::uniform_int_distribution<int> urd;
    return urd(getRng(), decltype(urd)::param_type{ low, high });
  }
  // EO getRandomInt

  // Create a random `float` between [low] and [high]
  float getRandomFloat(float low, float high)
  {
    static std::uniform_real_distribution<float> urd;
    return urd(getRng(), decltype(urd)::param_type{ low, high });
  }
  // EO getRandomFloat

  // Create a random `double` between [low] and [high]
  double getRandomDouble(double low, double high)
  {
    static std::uniform_real_distribution<double> urd;
    return urd(getRng(), decltype(urd)::param_type{ low, high });
  }
  // EO getRandomDouble

  // Get full 32bit random data
  uint32_t getRandomUint32()
  {
    return getRng()();
  }
  // EO getRandomUint32

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
