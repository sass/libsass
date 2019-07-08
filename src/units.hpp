/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_UNITS_HPP
#define SASS_UNITS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_def_macros.hpp"

namespace Sass {

  const double PI = std::acos(-1);

  enum UnitClass {
    LENGTH = 0x000,
    ANGLE = 0x100,
    TIME = 0x200,
    FREQUENCY = 0x300,
    RESOLUTION = 0x400,
    INCOMMENSURABLE = 0x500
  };

  enum UnitType {

    // size units
    INCH = UnitClass::LENGTH,
    CM,
    PC,
    MM,
    PT,
    PX,

    // angle units
    DEG = ANGLE,
    GRAD,
    RAD,
    TURN,

    // time units
    SEC = TIME,
    MSEC,

    // frequency units
    HERTZ = FREQUENCY,
    KHERTZ,

    // resolutions units
    DPI = RESOLUTION,
    DPCM,
    DPPX,

    // for unknown units
    UNKNOWN = INCOMMENSURABLE

  };

  class Units {

  private:

    mutable sass::string stringified;

  public:

    sass::vector<sass::string> numerators;
    sass::vector<sass::string> denominators;

    // default constructor
    Units() :
      numerators(),
      denominators()
    { }

    Units(const sass::string& u) :
      numerators(),
      denominators()
    {
      unit(u);
    }


    // copy constructor
    Units(const Units* ptr) :
      numerators(ptr->numerators),
      denominators(ptr->denominators)
    { }

    // copy constructor
    Units(const Units& ptr) :
      numerators(ptr.numerators),
      denominators(ptr.denominators)
    { }

    // move constructor
    Units(Units&& other) noexcept :
      numerators(std::move(other.numerators)),
      denominators(std::move(other.denominators))
    { }

    // convert to string
    const sass::string& unit() const;
    void unit(const sass::string& unit);
    // get if units are empty
    bool isUnitless() const;

    bool hasUnits() const {
      return !isUnitless();
    }
    // return if valid for css
    bool isValidCssUnit() const;
    // reduce units for output
    // returns conversion factor
    double reduce();
    // normalize units for compare
    // returns conversion factor
    double normalize();
    // compare operations
    bool operator==(const Units& rhs) const;
    // Delete other operators to make implementation more clear
    // Helps us spot cases where we use undefined implementations
    // bool operator!=(const Units& rhs) const = delete;
    // bool operator>=(const Units& rhs) const = delete;
    // bool operator<=(const Units& rhs) const = delete;
    // bool operator>(const Units& rhs) const = delete;
    // bool operator<(const Units& rhs) const = delete;

    // factor to convert into given units
    double getUnitConvertFactor(const Units&) const;
    // 
    bool hasUnit(sass::string numerator);
  };

  /* Declare matrix tables for unit conversion factors*/
  extern const double size_conversion_factors[6][6];
  extern const double angle_conversion_factors[4][4];
  extern const double time_conversion_factors[2][2];
  extern const double frequency_conversion_factors[2][2];
  extern const double resolution_conversion_factors[3][3];

  UnitType get_standard_unit(const UnitClass unit);
  enum Sass::UnitType string_to_unit(const sass::string&);
  const char* unit_to_string(Sass::UnitType unit);
  enum Sass::UnitClass get_unit_type(Sass::UnitType unit);
  // throws incompatibleUnits exceptions
  double conversion_factor(const sass::string&, const sass::string&);
  double conversion_factor(UnitType, UnitType, UnitClass, UnitClass);
  double convert_units(const sass::string&, const sass::string&, int&, int&);

}

#endif
