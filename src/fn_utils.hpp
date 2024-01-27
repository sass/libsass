/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_FN_UTILS_HPP
#define SASS_FN_UTILS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// Make some macros available
#include "ast_fwd_decl.hpp" 

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Returns whether [lhs] and [rhs] are equal within [epsilon].
  inline bool fuzzyEquals(double lhs, double rhs, double epsilon) {
    return fabs(lhs - rhs) < epsilon;
  }

  // Returns whether [lhs] is less than [rhs], and not [fuzzyEquals].
  inline bool fuzzyLessThan(double lhs, double rhs, double epsilon) {
    return lhs < rhs && !fuzzyEquals(lhs, rhs, epsilon);
  }

  // Returns whether [lhs] is less than [rhs], or [fuzzyEquals].
  inline bool fuzzyLessThanOrEquals(double lhs, double rhs, double epsilon) {
    return lhs < rhs || fuzzyEquals(lhs, rhs, epsilon);
  }

  // Returns whether [lhs] is greater than [rhs], and not [fuzzyEquals].
  inline bool fuzzyGreaterThan(double lhs, double rhs, double epsilon) {
    return lhs > rhs && !fuzzyEquals(lhs, rhs, epsilon);
  }

  // Returns whether [lhs] is greater than [rhs], or [fuzzyEquals].
  inline bool fuzzyGreaterThanOrEquals(double lhs, double rhs, double epsilon) {
    return lhs > rhs || fuzzyEquals(lhs, rhs, epsilon);
  }

  // Returns whether [number] is [fuzzyEquals] to an integer.
  inline bool fuzzyIsInt(double number, double epsilon) {
    // Check against 0.5 rather than 0.0 so that we catch numbers that
    // are both very slightly above an integer, and very slightly below.
    double _fabs_ = fabs(number - 0.5);
    double _fmod_ = fmod(_fabs_, 1.0);
    return fuzzyEquals(_fmod_, 0.5, epsilon);
  }

  // Rounds [number] to the nearest integer.
  // This rounds up numbers that are [fuzzyEquals] to `X.5`.
  inline long fuzzyRound(double number, double epsilon) {
    // If the number is within epsilon of X.5,
    // round up (or down for negative numbers).
    if (number > 0) {
      return lround(fuzzyLessThan(
        fmod(number, 1.0), 0.5, epsilon)
        ? floor(number) : ceill(number));
    }
    return lround(fuzzyLessThanOrEquals(
      fmod(number, 1.0), -0.5, epsilon)
      ? floorl(number) : ceill(number));
  }

  // Returns `true` if it's within [min] and [max],
  // or [number] is [fuzzyEquals] to [min] or [max].
  inline bool fuzzyCheckRange(double number, double min, double max, double epsilon)
  {
    return (number > min && number < max)
      || fuzzyEquals(number, min, epsilon)
      || fuzzyEquals(number, max, epsilon);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  inline double round64(double val, double epsilon)
  {
    // https://github.com/sass/sass/commit/4e3e1d5684cc29073a507578fc977434ff488c93
    // ToDo: maybe speed up further by using `std::remainder`
    double rest = std::fmod(val, 1.0) / 5.0;
    if (val >= 0) {
      if (0.1 - rest < epsilon) return std::ceil(val);
      else return std::floor(val);
    }
    if (rest + 0.1 <= epsilon) return std::floor(val);
    else return std::ceil(val);
  }

  template <typename T>
  inline T clamp(const T& n, const T& lower, const T& upper)
  {
    return std::max(lower, std::min(n, upper));
  }

  template <typename T>
  inline T absmod(const T& n, const T& r)
  {
    T m = std::fmod(n, r);
    if (m < 0.0) m += r;
    return m;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  #define FN_PROTOTYPE \
    const SourceSpan& pstate, \
    const ValueVector& arguments, \
    Compiler& compiler, \
    Eval& eval \

  #define BUILT_IN_FN(name) Value* name(FN_PROTOTYPE)

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

};

#endif
