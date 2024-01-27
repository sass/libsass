/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "units.hpp"

#include "flat_map.hpp"
#include "string_utils.hpp"

#include <set>

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /* the conversion matrix can be read the following way */
  /* if you go down, the factor is for the numerator (multiply) */
  /* if you go right, the factor is for the denominator (divide) */
  /* and yes, we actually use both, not sure why, but why not!? */
  /////////////////////////////////////////////////////////////////////////

  const int size_conversion_factors_count = 7;
  const double size_conversion_factors[7][7] =
  {
             /*  in           cm           pc           mm           pt           px           q           */
    /* in   */ { 1.0,         2.54,        6.0,         25.4,        72.0,        96.0,        101.6       },
    /* cm   */ { 1.0/2.54,    2.54/2.54,   6.0/2.54,    25.4/2.54,   72.0/2.54,   96.0/2.54,   101.6/2.54  },
    /* pc   */ { 1.0/6.0,     2.54/6.0,    6.0/6.0,     25.4/6.0,    72.0/6.0,    96.0/6.0,    101.6/6.0   },
    /* mm   */ { 1.0/25.4,    2.54/25.4,   6.0/25.4,    25.4/25.4,   72.0/25.4,   96.0/25.4,   101.6/25.4  },
    /* pt   */ { 1.0/72.0,    2.54/72.0,   6.0/72.0,    25.4/72.0,   72.0/72.0,   96.0/72.0,   101.6/72.0  },
    /* px   */ { 1.0/96.0,    2.54/96.0,   6.0/96.0,    25.4/96.0,   72.0/96.0,   96.0/96.0,   101.6/96.0  },
    /* q    */ { 1.0/101.6,   2.54/101.6,  6.0/101.6,   25.4/101.6,  72.0/101.6,  96.0/101.6,  101.6/101.6 }
  };

  const int time_conversion_factors_count = 2;
  const double time_conversion_factors[2][2] =
  {
             /*  s            ms        */
    /* s    */ { 1.0,         1000.0    },
    /* ms   */ { 1/1000.0,    1.0       }
  };

  const int angle_conversion_factors_count = 4;
  const double angle_conversion_factors[4][4] =
  {
             /*  deg          grad         rad          turn      */
    /* deg  */ { 1.0,         40.0/36.0,   PI/180.0,    1.0/360.0 },
    /* grad */ { 36.0/40.0,   1.0,         PI/200.0,    1.0/400.0 },
    /* rad  */ { 180.0/PI,    200.0/PI,    1.0,         0.5/PI    },
    /* turn */ { 360.0,       400.0,       2.0*PI,      1.0       }
  };

  const int frequency_conversion_factors_count = 2;
  const double frequency_conversion_factors[2][2] =
  {
             /*  Hz           kHz       */
    /* Hz   */ { 1.0,         1/1000.0  },
    /* kHz  */ { 1000.0,      1.0       }
  };

  const int resolution_conversion_factors_count = 3;
  const double resolution_conversion_factors[3][3] =
  {
             /*  dpi          dpcm         dppx     */
    /* dpi  */ { 1.0,         1.0/2.54,    1.0/96.0 },
    /* dpcm */ { 2.54,        1.0,         2.54/96  },
    /* dppx */ { 96.0,        96.0/2.54,   1.0      }
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return unit class enum for given unit type enum
  UnitClass get_unit_class(UnitType unit)
  {
    switch (unit & 0xFF00)
    {
      case UnitClass::LENGTH:      return UnitClass::LENGTH;
      case UnitClass::ANGLE:       return UnitClass::ANGLE;
      case UnitClass::TIME:        return UnitClass::TIME;
      case UnitClass::FREQUENCY:   return UnitClass::FREQUENCY;
      case UnitClass::RESOLUTION:  return UnitClass::RESOLUTION;
      default:                     return UnitClass::INCOMMENSURABLE;
    }
  };

  // Return standard unit for the given unit class enum
  UnitType get_standard_unit(const UnitClass unit)
  {
    switch (unit)
    {
      case UnitClass::LENGTH:      return UnitType::PX;
      case UnitClass::TIME:        return UnitType::SEC;
      case UnitClass::ANGLE:       return UnitType::DEG;
      case UnitClass::FREQUENCY:   return UnitType::HERTZ;
      case UnitClass::RESOLUTION:  return UnitType::DPI;
      default:                     return UnitType::UNKNOWN;
    }
  };

  // Return unit type enum from unit string
  UnitType string_to_unit(const sass::string& s)
  {
    // size units
    if      (StringUtils::equalsIgnoreCase(s, "px", 2))   return UnitType::PX;
    else if (StringUtils::equalsIgnoreCase(s, "pt", 2))   return UnitType::PT;
    else if (StringUtils::equalsIgnoreCase(s, "pc", 2))   return UnitType::PC;
    else if (StringUtils::equalsIgnoreCase(s, "mm", 2))   return UnitType::MM;
    else if (StringUtils::equalsIgnoreCase(s, "cm", 2))   return UnitType::CM;
    else if (StringUtils::equalsIgnoreCase(s, "in", 2))   return UnitType::INCH;
    else if (StringUtils::equalsIgnoreCase(s, "q", 1))    return UnitType::QMM;
    // time units
    else if (StringUtils::equalsIgnoreCase(s, "s", 1))    return UnitType::SEC;
    else if (StringUtils::equalsIgnoreCase(s, "ms", 2))   return UnitType::MSEC;
    // angle units
    else if (StringUtils::equalsIgnoreCase(s, "deg", 3))  return UnitType::DEG;
    else if (StringUtils::equalsIgnoreCase(s, "grad", 4)) return UnitType::GRAD;
    else if (StringUtils::equalsIgnoreCase(s, "rad", 3))  return UnitType::RAD;
    else if (StringUtils::equalsIgnoreCase(s, "turn", 4)) return UnitType::TURN;
    // frequency units
    else if (StringUtils::equalsIgnoreCase(s, "hz", 2))   return UnitType::HERTZ;
    else if (StringUtils::equalsIgnoreCase(s, "khz", 3))  return UnitType::KHERTZ;
    // resolutions units
    else if (StringUtils::equalsIgnoreCase(s, "dpi", 3))  return UnitType::DPI;
    else if (StringUtils::equalsIgnoreCase(s, "dpcm", 4)) return UnitType::DPCM;
    else if (StringUtils::equalsIgnoreCase(s, "dppx", 4)) return UnitType::DPPX;
    // for unknown units
    else return UnitType::UNKNOWN;
  }

  // Return unit as string from unit type enum
  const char* unit_to_string(UnitType unit)
  {
    switch (unit) {
      // size units
      case UnitType::PX:      return "px";
      case UnitType::PT:      return "pt";
      case UnitType::PC:      return "pc";
      case UnitType::MM:      return "mm";
      case UnitType::CM:      return "cm";
      case UnitType::INCH:    return "in";
      case UnitType::QMM:     return "q";
      // time units
      case UnitType::SEC:     return "s";
      case UnitType::MSEC:    return "ms";
      // angle units
      case UnitType::DEG:     return "deg";
      case UnitType::GRAD:    return "grad";
      case UnitType::RAD:     return "rad";
      case UnitType::TURN:    return "turn";
      // frequency units
      case UnitType::HERTZ:   return "Hz";
      case UnitType::KHERTZ:  return "kHz";
      // resolutions units
      case UnitType::DPI:     return "dpi";
      case UnitType::DPCM:    return "dpcm";
      case UnitType::DPPX:    return "dppx";
      // for unknown units
      default:                return "";
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return conversion factor from s1 to s2 (returns zero for incompatible units)
  double conversion_factor(const sass::string& s1, const sass::string& s2)
  {
    // assert for same units
    if (s1 == s2) return 1;
    // get unit enum from string
    UnitType u1 = string_to_unit(s1);
    UnitType u2 = string_to_unit(s2);
    // query unit group types
    UnitClass t1 = get_unit_class(u1);
    UnitClass t2 = get_unit_class(u2);
    // return the conversion factor
    return conversion_factor(u1, u2, t1, t2);
  }

  // Return conversion factor from u1 to u2 (returns zero for incompatible units)
  // Note: unit classes are passed as parameters since we mostly already have them
  // Note: not sure how much performance this saves, but it fits our use-cases well
  double conversion_factor(UnitType u1, UnitType u2, UnitClass t1, UnitClass t2)
  {
    // can't convert different classes
    if (t1 != t2) return 0;
    // get absolute offset for array access
    size_t i1 = u1 & 0x00FF;
    size_t i2 = u2 & 0x00FF;
    // process known units
    switch (t1) {
      case LENGTH:
        if (i1 >= size_conversion_factors_count) return 0.0;
        if (i2 >= size_conversion_factors_count) return 0.0;
        return size_conversion_factors[i1][i2];
      case TIME:
        if (i1 >= time_conversion_factors_count) return 0.0;
        if (i2 >= time_conversion_factors_count) return 0.0;
        return time_conversion_factors[i1][i2];
      case ANGLE:
        if (i1 >= angle_conversion_factors_count) return 0.0;
        if (i2 >= angle_conversion_factors_count) return 0.0;
        return angle_conversion_factors[i1][i2];
      case FREQUENCY:
        if (i1 >= frequency_conversion_factors_count) return 0.0;
        if (i2 >= frequency_conversion_factors_count) return 0.0;
        return frequency_conversion_factors[i1][i2];
      case RESOLUTION:
        if (i1 >= resolution_conversion_factors_count) return 0.0;
        if (i2 >= resolution_conversion_factors_count) return 0.0;
        return resolution_conversion_factors[i1][i2];
      case INCOMMENSURABLE:
        return 0;
    }
    // fall-back
    return 0;
  }

  // Reduce units so that the result either is fully represented by lhs or rhs unit.
  // Exponents are adjusted accordingly and returning factor must be applied to the scalar.
  double reduce_units(const sass::string& lhs, const sass::string& rhs, int& lhsexp, int& rhsexp)
  {
    double f = 0;
    // do not convert same ones
    if (lhs == rhs) return 0.0;
    // skip already canceled out unit
    if (lhsexp == 0) return 0.0;
    if (rhsexp == 0) return 0.0;
    // check if it can be converted
    UnitType ulhs = string_to_unit(lhs);
    UnitType urhs = string_to_unit(rhs);
    // skip units we cannot convert
    if (ulhs == UNKNOWN) return 0.0;
    if (urhs == UNKNOWN) return 0.0;
    // query unit group types
    UnitClass clhs = get_unit_class(ulhs);
    UnitClass crhs = get_unit_class(urhs);
    // skip units we cannot convert
    if (clhs != crhs) return 0.0;
    // if right denominator is bigger than lhs, we want to keep it in rhs unit
    if (rhsexp < 0 && lhsexp > 0 && - rhsexp > lhsexp) {
      // get the conversion factor for units
      f = conversion_factor(urhs, ulhs, clhs, crhs);
      // left hand side has been consumed
      f = std::pow(f, lhsexp);
      rhsexp += lhsexp;
      lhsexp = 0;
    }
    else {
      // get the conversion factor for units
      f = conversion_factor(ulhs, urhs, clhs, crhs);
      // right hand side has been consumed
      f = std::pow(f, rhsexp);
      lhsexp += rhsexp;
      rhsexp = 0;
    }
    return f;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Compare units (without any normalizing)
  bool Units::operator== (const Units& rhs) const
  {
    return (numerators == rhs.numerators) &&
           (denominators == rhs.denominators);
  }

  // Normalize all units to the standard unit class
  // Additionally sorts all units in ascending order
  // In combination with `reduce` this allows numbers
  // to be compared for equality independent of units
  // E.g. '1000ms' will be normalized to '1s'
  // Returns factor to be applied to scalar
  double Units::normalize()
  {

    stringified.clear();
    size_t iL = numerators.size();
    size_t nL = denominators.size();

    // the final conversion factor
    double factor = 1.0;

    for (size_t i = 0; i < iL; i++) {
      sass::string &lhs = numerators[i];
      UnitType ulhs = string_to_unit(lhs);
      if (ulhs == UNKNOWN) continue;
      UnitClass clhs = get_unit_class(ulhs);
      UnitType umain = get_standard_unit(clhs);
      if (ulhs == umain) continue;
      double f(conversion_factor(umain, ulhs, clhs, clhs));
      if (f == 0) throw std::runtime_error("INVALID");
      numerators[i] = unit_to_string(umain);
      factor /= f;
    }

    for (size_t n = 0; n < nL; n++) {
      sass::string &rhs = denominators[n];
      UnitType urhs = string_to_unit(rhs);
      if (urhs == UNKNOWN) continue;
      UnitClass crhs = get_unit_class(urhs);
      UnitType umain = get_standard_unit(crhs);
      if (urhs == umain) continue;
      // this is never hit via spec-tests!?
      double f(conversion_factor(umain, urhs, crhs, crhs));
      if (f == 0) throw std::runtime_error("INVALID");
      denominators[n] = unit_to_string(umain);
      factor /= f;
    }

    std::sort (numerators.begin(), numerators.end());
    std::sort (denominators.begin(), denominators.end());

    // return for conversion
    return factor;
  }
  // EO normalize

  // Cancel out all compatible unit classes
  // E.g. `1000ms/s` will be reduced to `1`
  // Returns factor to be applied to scalar
  double Units::reduce()
  {

    stringified.clear();
    size_t iL = numerators.size();
    size_t nL = denominators.size();

    // have less than two units?
    if (iL + nL < 2) return 1;

    // first make sure same units cancel each other out
    // it seems that a map table will fit nicely to do this
    // we basically construct exponents for each unit
    // has the advantage that they will be presorted
    // ToDo: use fast map implementation?
    FlatMap<sass::string, int> exponents;

    // initialize by summing up occurrences in unit vectors
    // this will already cancel out equivalent units (e.q. px/px)
    for (size_t i = 0; i < iL; i ++) exponents[numerators[i]] += 1;
    for (size_t n = 0; n < nL; n ++) exponents[denominators[n]] -= 1;

    // the final conversion factor
    double factor = 1;

    // convert between compatible units
    for (size_t i = 0; i < iL; i++) {
      for (size_t n = 0; n < nL; n++) {
        sass::string &lhs = numerators[i], &rhs = denominators[n];
        int &lhsexp = exponents[lhs], &rhsexp = exponents[rhs];
        double f(reduce_units(lhs, rhs, lhsexp, rhsexp));
        if (f == 0) continue;
        factor /= f;
      }
    }

    // now we can build up the new unit arrays
    numerators.clear();
    denominators.clear();

    // recreate sorted units vectors
    for (auto kv : exponents) {
      int &exponent = kv.second;
      while (exponent > 0 && exponent --)
        numerators.emplace_back(kv.first);
      while (exponent < 0 && exponent ++)
        denominators.emplace_back(kv.first);
    }

    // return for conversion
    return factor;

  }
  // EO reduce

  // Reset unit without conversion factor
  void Units::unit(const sass::string& u)
  {
    size_t l = 0;
    size_t r;
    stringified.clear();
    numerators.clear();
    denominators.clear();
    if (!u.empty()) {
      bool nominator = true;
      while (true) {
        r = u.find_first_of("*/", l);
        sass::string unit(u.substr(l, r == sass::string::npos ? r : r - l));
        if (!unit.empty()) {
          if (nominator) numerators.emplace_back(unit);
          else denominators.emplace_back(unit);
        }
        if (r == sass::string::npos) break;
        // ToDo: should error for multiple slashes
        // if (!nominator && u[r] == '/') error(...)
        if (u[r] == '/')
          nominator = false;
        // strange math parsing?
        // else if (u[r] == '*')
        //  nominator = true;
        l = r + 1;
      }
    }
  }
  // EO unit

  // Convert units to string
  const sass::string& Units::unit() const
  {
    // Units are expected to be short, so we hopefully
    // can profit from small objects optimization. This
    // is not guaranteed, but still safe to assume that
    // any mature implementation utilizes it.
    if (stringified.empty()) {
      size_t iL = numerators.size();
      size_t nL = denominators.size();
      for (size_t i = 0; i < iL; i += 1) {
        if (i) stringified += '*';
        stringified += numerators[i];
      }
      if (iL == 0) {
        if (nL > 1) stringified += '(';
        for (size_t n = 0; n < nL; n += 1) {
          if (n) stringified += '*';
          stringified += denominators[n];
        }
        if (nL > 1) stringified += ')';
        if (nL != 0) stringified += "^-1";
      }
      else {
        if (nL != 0) stringified += '/';
        for (size_t n = 0; n < nL; n += 1) {
          if (n) stringified += '*';
          stringified += denominators[n];
        }
      }
    }
    /*

    if (stringified.empty()) {
      size_t iL = numerators.size();
      size_t nL = denominators.size();
      if (iL > 0) {
        stringified += numerators[0];
      }
      for (size_t i = 1; i < iL; i += 1) {
        stringified += " * 1";
        stringified += numerators[i];
      }
      for (size_t n = 0; n < nL; n += 1) {
        stringified += " / 1";
        stringified += denominators[n];
      }
    }

    */
    return stringified;
  }
  // EO unit

    // Convert units to string
  const sass::string& Units::unit2() const
  {
    if (stringified.empty()) {
      size_t iL = numerators.size();
      size_t nL = denominators.size();
      if (iL > 0) {
        stringified += numerators[0];
      }
      for (size_t i = 1; i < iL; i += 1) {
        stringified += " * 1";
        stringified += numerators[i];
      }
      for (size_t n = 0; n < nL; n += 1) {
        stringified += " / 1";
        stringified += denominators[n];
      }
    }

    return stringified;
  }
  // EO unit
  // Returns true if we only have given numerator
  bool Units::isOnlyOfUnit(sass::string unit) const
  {
    return numerators.size() == 1 &&
      denominators.empty() &&
      numerators[0] == unit;
  }
  // EO isOnlyOfUnit

  // Returns true if empty
  bool Units::isUnitless() const
  {
    return numerators.empty() &&
           denominators.empty();
  }
  // EO isUnitless

  // Returns true if valid for css
  bool Units::isValidCssUnit() const
  {
    return numerators.size() <= 1 &&
           denominators.size() == 0;
  }
  // EO isValidCssUnit

  const std::set<sass::string> KnownUnits({
    "em", "rem", "ex", "rex", "cap", "rcap", "ch", "rch", "ic", "ric", "lh", //
    "rlh", "vw", "lvw", "svw", "dvw", "vh", "lvh", "svh", "dvh", "vi", "lvi", //
    "svi", "dvi", "vb", "lvb", "svb", "dvb", "vmin", "lvmin", "svmin", //
    "dvmin", "vmax", "lvmax", "svmax", "dvmax", "cqw", "cqh", "cqi", "cqb", //
    "cqmin", "cqmax", "cm", "mm", "q", "in", "pt", "pc", "px",
    "deg", "grad", "rad", "turn",
    "dpi", "dpcm", "dppx",
    "hz", "khz",
    "s", "ms",
  });

  const std::set<sass::string> CompatUnitsLen({
    "em", "rem", "ex", "rex", "cap", "rcap", "ch", "rch", "ic", "ric", "lh", //
    "rlh", "vw", "lvw", "svw", "dvw", "vh", "lvh", "svh", "dvh", "vi", "lvi", //
    "svi", "dvi", "vb", "lvb", "svb", "dvb", "vmin", "lvmin", "svmin", //
    "dvmin", "vmax", "lvmax", "svmax", "dvmax", "cqw", "cqh", "cqi", "cqb", //
    "cqmin", "cqmax", "cm", "mm", "q", "in", "pt", "pc", "px"
  });

  const std::set<sass::string> CompatUnitsAngle({ "deg", "grad", "rad", "turn" });
  const std::set<sass::string> CompatUnitsDPI({ "dpi", "dpcm", "dppx" });
  const std::set<sass::string> CompatUnitsFreq({ "hz", "khz" });
  const std::set<sass::string> CompatUnitsTime({ "s", "ms" });

  const std::unordered_map<sass::string, std::set<sass::string>> CompatsByUnit({
    { "em", CompatUnitsLen }, { "rem", CompatUnitsLen }, { "ex", CompatUnitsLen }, { "rex", CompatUnitsLen }, { "cap", CompatUnitsLen },
    { "rcap", CompatUnitsLen }, { "ch", CompatUnitsLen }, { "rch", CompatUnitsLen }, { "ic", CompatUnitsLen }, { "ric", CompatUnitsLen },
    { "lh", CompatUnitsLen }, { "rlh", CompatUnitsLen }, { "vw", CompatUnitsLen }, { "lvw", CompatUnitsLen }, { "svw", CompatUnitsLen },
    { "dvw", CompatUnitsLen }, { "vh", CompatUnitsLen }, { "lvh", CompatUnitsLen }, { "svh", CompatUnitsLen }, { "dvh", CompatUnitsLen },
    { "vi", CompatUnitsLen }, { "lvi", CompatUnitsLen }, { "svi", CompatUnitsLen }, { "dvi", CompatUnitsLen }, { "vb", CompatUnitsLen },
    { "lvb", CompatUnitsLen }, { "svb", CompatUnitsLen }, { "dvb", CompatUnitsLen }, { "vmin", CompatUnitsLen }, { "lvmin", CompatUnitsLen },
    { "svmin", CompatUnitsLen }, { "dvmin", CompatUnitsLen }, { "vmax", CompatUnitsLen }, { "lvmax", CompatUnitsLen }, { "svmax", CompatUnitsLen },
    { "dvmax", CompatUnitsLen }, { "cqw", CompatUnitsLen }, { "cqh", CompatUnitsLen }, { "cqi", CompatUnitsLen }, { "cqb", CompatUnitsLen },
    { "cqmin", CompatUnitsLen }, { "cqmax", CompatUnitsLen }, { "cm", CompatUnitsLen }, { "mm", CompatUnitsLen }, { "q", CompatUnitsLen },
    { "in", CompatUnitsLen }, { "pt", CompatUnitsLen }, { "pc", CompatUnitsLen }, { "px", CompatUnitsLen },
    { "deg", CompatUnitsAngle }, { "grad", CompatUnitsAngle }, { "rad", CompatUnitsAngle }, { "turn", CompatUnitsAngle },
    { "dpi", CompatUnitsDPI }, { "dpcm", CompatUnitsDPI }, { "dppx", CompatUnitsDPI },
    { "hz", CompatUnitsFreq }, { "khz", CompatUnitsFreq },
    { "s", CompatUnitsTime }, { "ms", CompatUnitsTime },
  });

  static bool isaCustomUnit(const sass::string& unit) {
    sass::string norm(StringUtils::toLowerCase(unit));
    if (KnownUnits.count(norm)) return false;
    return string_to_unit(unit) == UnitType::UNKNOWN;
  }

  // Returns true if unit is "unknown"
  // Meaning we don't know to convert it
  bool Units::isCustomUnit() const
  {
    for (auto n : numerators) if (isaCustomUnit(n)) return true;
    for (auto d : denominators) if (isaCustomUnit(d)) return true;
    return false;
  }
  // EO isCustomUnit

  bool Units::canCompareTo(const Units& r, bool legacy) const
  {
    if (legacy) return isComparableTo(r);
    else return hasCompatibleUnits(r);
  }

  bool Units::isComparableTo(const Units& r) const
  {
    if (numerators.size() != r.numerators.size()) return false;
    if (denominators.size() != r.denominators.size()) return false;
    return getUnitConversionFactor(r, false) != 0;
  }

  bool Units::hasCompatibleUnits(const Units& r, bool strict) const
  {
    return getUnitConversionFactor(r, strict) != 0;
  }

  // vh and px are possibly compatible, although we can't reduce on compile time
  bool Units::hasPossiblyCompatibleUnits(const Units& r, bool strict) const
  {
    // If equal they are compatible
    if (r == this) return true;
    // Otherwise check our compatibility sets
    if (r.numerators.size() == 1 && r.denominators.empty()) {
      if (numerators.size() == 1 && denominators.empty()) {
        sass::string lu(StringUtils::toLowerCase(numerators[0]));
        sass::string ru(StringUtils::toLowerCase(r.numerators[0]));
        // Most simple case where both have exactly one unit
        auto kv = CompatsByUnit.find(lu);
        // Unit has no known conversion possible
        if (kv == CompatsByUnit.end()) return true;
        // Check if second unit is fully compatible
        if (kv->second.count(ru)) return true;
        return CompatsByUnit.count(ru) == 0;

      }
    }
    if (r.numerators.empty() && r.denominators.empty()) {
      return false;
    }
    if (numerators.empty() && denominators.size() == 0) {
      return false;
    }
    std::cerr << "case not implemented\n";
    return false;
  }

  // Return factor to convert into passed units
  double Units::getUnitConversionFactor(const Units& r, bool strict) const
  {

    sass::vector<sass::string> miss_nums(0);
    sass::vector<sass::string> miss_dens(0);
    // create copy since we need these for state keeping
    sass::vector<sass::string> r_nums(r.numerators);
    sass::vector<sass::string> r_dens(r.denominators);

    auto l_num_it = numerators.begin();
    auto l_num_end = numerators.end();

    bool l_unitless = isUnitless();
    auto r_unitless = r.isUnitless();

    if (strict)
    {
      if (!l_unitless && r_unitless) return 0.0;
      if (l_unitless && !r_unitless) return 0.0;
    }

    // overall conversion
    double factor = 1;

    // process all left numerators
    while (l_num_it != l_num_end)
    {
      // get and increment afterwards
      const sass::string l_num = *(l_num_it ++);

      // ToDo: we erase from base vector in the loop.
      // Iterators might get invalid during the loop
      // ToDo: refactor to use index access instead.
      auto r_num_it = r_nums.begin(), r_num_end = r_nums.end();

      bool found = false;
      // search for compatible numerator
      while (r_num_it != r_num_end)
      {
        // get and increment afterwards
        const sass::string r_num = *(r_num_it);
        // get possible conversion factor for units
        double conversion = conversion_factor(l_num, r_num);
        // skip incompatible numerator
        if (conversion == 0) {
          ++ r_num_it;
          continue;
        }
        // apply to global factor
        factor *= conversion;
        // remove item from vector
        r_nums.erase(r_num_it);
        // found numerator
        found = true;
        break;
      }
      // maybe we did not find any
      // left numerator is leftover
      if (!found) miss_nums.emplace_back(l_num);
    }

    auto l_den_it = denominators.begin();
    auto l_den_end = denominators.end();

    // process all left denominators
    while (l_den_it != l_den_end)
    {
      // get and increment afterwards
      const sass::string l_den = *(l_den_it ++);

      auto r_den_it = r_dens.begin();
      auto r_den_end = r_dens.end();

      bool found = false;
      // search for compatible denominator
      while (r_den_it != r_den_end)
      {
        // get and increment afterwards
        const sass::string r_den = *(r_den_it);
        // get possible conversion factor for units
        double conversion = conversion_factor(l_den, r_den);
        // skip incompatible denominator
        if (conversion == 0) {
          ++ r_den_it;
          continue;
        }
        // apply to global factor
        factor /= conversion;
        // remove item from vector
        r_dens.erase(r_den_it);
        // found denominator
        found = true;
        break;
      }
      // maybe we did not find any
      // left denominator is leftover
      if (!found) miss_dens.emplace_back(l_den);
    }

    // check left-overs (ToDo: might cancel out?)
    if (miss_nums.size() > 0 && !r_unitless) {
      return 0.0;
    }
    else if (miss_dens.size() > 0 && !r_unitless) {
      return 0.0;
    }
    else if (r_nums.size() > 0 && !l_unitless) {
      return 0.0;
    }
    else if (r_dens.size() > 0 && !l_unitless) {
      return 0.0;
    }

    return factor;
  }
  // EO getUnitConversionFactor

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
