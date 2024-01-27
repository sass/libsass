/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_UNITS_HPP
#define SASS_UNITS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  enum UnitClass {
    LENGTH = 0x000,
    TIME = 0x100,
    ANGLE = 0x200,
    FREQUENCY = 0x300,
    RESOLUTION = 0x400,
    INCOMMENSURABLE = 0x500
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  enum UnitType {

    // size units
    INCH = UnitClass::LENGTH,
    CM,
    PC,
    MM,
    PT,
    PX,
    QMM,

    // time units
    SEC = UnitClass::TIME,
    MSEC,

    // angle units
    DEG = UnitClass::ANGLE,
    GRAD,
    RAD,
    TURN,

    // frequency units
    HERTZ = UnitClass::FREQUENCY,
    KHERTZ,

    // resolutions units
    DPI = UnitClass::RESOLUTION,
    DPCM,
    DPPX,

    // for unknown units
    UNKNOWN = UnitClass::INCOMMENSURABLE

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class Units {

  private:

    // Cache the final unit string
    mutable sass::string stringified;

  public:

    // The units in the numerator
    sass::vector<sass::string> numerators;
    // The units in the denominator
    sass::vector<sass::string> denominators;

    // Default constructor
    Units() :
      numerators(),
      denominators()
    { }

    // Construct from string 
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

    // Convert units to string
    const sass::string& unit() const;

    // Reset unit without conversion factor
    void unit(const sass::string& unit);

    // Returns true if empty
    bool isUnitless() const;

    // Returns true if not empty
    bool hasUnits() const {
      return !isUnitless();
    }

    const sass::string& unit2() const;

    // Returns true if we only have given numerator
    bool isOnlyOfUnit(sass::string numerator) const;

    // Returns true if valid for css
    bool isValidCssUnit() const;

    // Cancel out all compatible unit classes
    // E.g. `1000ms/s` will be reduced to `1`
    // Returns factor to be applied to scalar
    double reduce();

    // Normalize all units to the standard unit class
    // Additionally sorts all units in ascending order
    // In combination with `reduce` this allows numbers
    // to be compared for equality independent of units
    // E.g. '1000ms' will be normalized to '1s'
    // Returns factor to be applied to scalar
    double normalize();

    // Compare units (without any normalizing)
    bool operator==(const Units& rhs) const;

    // Delete other operators to make implementation more clear
    // Helps us spot cases where we use undefined implementations
    // bool operator!=(const Units& rhs) const = delete;
    // bool operator>=(const Units& rhs) const = delete;
    // bool operator<=(const Units& rhs) const = delete;
    // bool operator>(const Units& rhs) const = delete;
    // bool operator<(const Units& rhs) const = delete;

    // Returns true if unit is "unknown"
    // Meaning we don't know to convert it
    bool isCustomUnit() const;

    bool canCompareTo(const Units&, bool) const;

    bool isComparableTo(const Units&) const;

    // Return if conversion between units is possible
    bool hasCompatibleUnits(const Units&, bool strict = false) const;

    // Returns whether [this] has units that are possibly
    // compatible with [rhs], as defined by the Sass spec.
    bool hasPossiblyCompatibleUnits(const Units&, bool strict = false) const;

    // Return factor to convert into passed units
    double getUnitConversionFactor(const Units&, bool strict = false) const;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /* Declare matrix tables for unit conversion factors*/
  extern const double size_conversion_factors[7][7];
  extern const double angle_conversion_factors[4][4];
  extern const double time_conversion_factors[2][2];
  extern const double frequency_conversion_factors[2][2];
  extern const double resolution_conversion_factors[3][3];

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return unit class enum for given unit type enum
  UnitClass get_unit_class(UnitType unit);

  // Return standard unit for the given unit class enum
  UnitType get_standard_unit(const UnitClass unit);

  // Return unit type enum from unit string
  UnitType string_to_unit(const sass::string& s);

  // Return unit as string from unit type enum
  const char* unit_to_string(UnitType unit);

  // Return conversion factor from s1 to s2 (returns zero for incompatible units)
  double conversion_factor(const sass::string& s1, const sass::string& s2);

  // Return conversion factor from u1 to u2 (returns zero for incompatible units)
  // Note: unit classes are passed as parameters since we mostly already have them
  // Note: not sure how much performance this saves, but it fits our use-cases well
  double conversion_factor(UnitType u1, UnitType u2, UnitClass c1, UnitClass c2);

  // Reduce units so that the result either is fully represented by lhs or rhs unit.
  // Exponents are adjusted accordingly and returning factor must be applied to the scalar.
  // Basically tries to cancel out compatible units (e.g. s/ms) and converts the remaining ones.
  double reduce_units(const sass::string& lhs, const sass::string& rhs, int& lhsexp, int& rhsexp);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
