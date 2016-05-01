#include "../sass.hpp"
#include "values.hpp"
#include "../ast.hpp"
#include "../context.hpp"
#include "../node.hpp"
#include "../extend.hpp"
#include "../emitter.hpp"
#include "../color_maps.hpp"
#include <set>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace Sass {


  Number::Number(ParserState pstate, double val, std::string u, bool zero)
  : Value(pstate),
    value_(val),
    zero_(zero),
    numerator_units_(std::vector<std::string>()),
    denominator_units_(std::vector<std::string>()),
    hash_(0)
  {
    size_t l = 0, r = 0;
    if (!u.empty()) {
      bool nominator = true;
      while (true) {
        r = u.find_first_of("*/", l);
        std::string unit(u.substr(l, r == std::string::npos ? r : r - l));
        if (!unit.empty()) {
          if (nominator) numerator_units_.push_back(unit);
          else denominator_units_.push_back(unit);
        }
        if (r == std::string::npos) break;
        // ToDo: should error for multiple slashes
        // if (!nominator && u[r] == '/') error(...)
        if (u[r] == '/')
          nominator = false;
        l = r + 1;
      }
    }
    concrete_type(NUMBER);
  }

  std::string Number::unit() const
  {
    std::string u;
    for (size_t i = 0, S = numerator_units_.size(); i < S; ++i) {
      if (i) u += '*';
      u += numerator_units_[i];
    }
    if (!denominator_units_.empty()) u += '/';
    for (size_t i = 0, S = denominator_units_.size(); i < S; ++i) {
      if (i) u += '*';
      u += denominator_units_[i];
    }
    return u;
  }

  bool Number::is_valid_css_unit() const
  {
    return numerator_units().size() <= 1 &&
           denominator_units().size() == 0;
  }

  bool Number::is_unitless() const
  { return numerator_units_.empty() && denominator_units_.empty(); }

  void Number::normalize(const std::string& prefered, bool strict)
  {

    // first make sure same units cancel each other out
    // it seems that a map table will fit nicely to do this
    // we basically construct exponents for each unit
    // has the advantage that they will be pre-sorted
    std::map<std::string, int> exponents;

    // initialize by summing up occurences in unit vectors
    for (size_t i = 0, S = numerator_units_.size(); i < S; ++i) ++ exponents[numerator_units_[i]];
    for (size_t i = 0, S = denominator_units_.size(); i < S; ++i) -- exponents[denominator_units_[i]];

    // the final conversion factor
    double factor = 1;

    // get the first entry of numerators
    // forward it when entry is converted
    std::vector<std::string>::iterator nom_it = numerator_units_.begin();
    std::vector<std::string>::iterator nom_end = numerator_units_.end();
    std::vector<std::string>::iterator denom_it = denominator_units_.begin();
    std::vector<std::string>::iterator denom_end = denominator_units_.end();

    // main normalization loop
    // should be close to optimal
    while (denom_it != denom_end)
    {
      // get and increment afterwards
      const std::string denom = *(denom_it ++);
      // skip already canceled out unit
      if (exponents[denom] >= 0) continue;
      // skip all units we don't know how to convert
      if (string_to_unit(denom) == UNKNOWN) continue;
      // now search for nominator
      while (nom_it != nom_end)
      {
        // get and increment afterwards
        const std::string nom = *(nom_it ++);
        // skip already canceled out unit
        if (exponents[nom] <= 0) continue;
        // skip all units we don't know how to convert
        if (string_to_unit(nom) == UNKNOWN) continue;
        // we now have two convertable units
        // add factor for current conversion
        factor *= conversion_factor(nom, denom, strict);
        // update nominator/denominator exponent
        -- exponents[nom]; ++ exponents[denom];
        // inner loop done
        break;
      }
    }

    // now we can build up the new unit arrays
    numerator_units_.clear();
    denominator_units_.clear();

    // build them by iterating over the exponents
    for (auto exp : exponents)
    {
      // maybe there is more effecient way to push
      // the same item multiple times to a vector?
      for(size_t i = 0, S = abs(exp.second); i < S; ++i)
      {
        // opted to have these switches in the inner loop
        // makes it more readable and should not cost much
        if (!exp.first.empty()) {
          if (exp.second < 0) denominator_units_.push_back(exp.first);
          else if (exp.second > 0) numerator_units_.push_back(exp.first);
        }
      }
    }

    // apply factor to value_
    // best precision this way
    value_ *= factor;

    // maybe convert to other unit
    // easier implemented on its own
    try { convert(prefered, strict); }
    catch (incompatibleUnits& err)
    { error(err.what(), pstate()); }
    catch (...) { throw; }

  }

  // this does not cover all cases (multiple prefered units)
  double Number::convert_factor(const Number& n) const
  {

    // first make sure same units cancel each other out
    // it seems that a map table will fit nicely to do this
    // we basically construct exponents for each unit class
    // std::map<std::string, int> exponents;
    // initialize by summing up occurences in unit vectors
    // for (size_t i = 0, S = numerator_units_.size(); i < S; ++i) ++ exponents[unit_to_class(numerator_units_[i])];
    // for (size_t i = 0, S = denominator_units_.size(); i < S; ++i) -- exponents[unit_to_class(denominator_units_[i])];

    std::vector<std::string> l_miss_nums(0);
    std::vector<std::string> l_miss_dens(0);
    // create copy since we need these for state keeping
    std::vector<std::string> r_nums(n.numerator_units_);
    std::vector<std::string> r_dens(n.denominator_units_);

    std::vector<std::string>::const_iterator l_num_it = numerator_units_.begin();
    std::vector<std::string>::const_iterator l_num_end = numerator_units_.end();

    bool l_unitless = is_unitless();
    bool r_unitless = n.is_unitless();

    // overall conversion
    double factor = 1;

    // process all left numerators
    while (l_num_it != l_num_end)
    {
      // get and increment afterwards
      const std::string l_num = *(l_num_it ++);

      std::vector<std::string>::iterator r_num_it = r_nums.begin();
      std::vector<std::string>::iterator r_num_end = r_nums.end();

      bool found = false;
      // search for compatible numerator
      while (r_num_it != r_num_end)
      {
        // get and increment afterwards
        const std::string r_num = *(r_num_it);
        // get possible converstion factor for units
        double conversion = conversion_factor(l_num, r_num, false);
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
      if (!found) l_miss_nums.push_back(l_num);
    }

    std::vector<std::string>::const_iterator l_den_it = denominator_units_.begin();
    std::vector<std::string>::const_iterator l_den_end = denominator_units_.end();

    // process all left denominators
    while (l_den_it != l_den_end)
    {
      // get and increment afterwards
      const std::string l_den = *(l_den_it ++);

      std::vector<std::string>::iterator r_den_it = r_dens.begin();
      std::vector<std::string>::iterator r_den_end = r_dens.end();

      bool found = false;
      // search for compatible denominator
      while (r_den_it != r_den_end)
      {
        // get and increment afterwards
        const std::string r_den = *(r_den_it);
        // get possible converstion factor for units
        double conversion = conversion_factor(l_den, r_den, false);
        // skip incompatible denominator
        if (conversion == 0) {
          ++ r_den_it;
          continue;
        }
        // apply to global factor
        factor *= conversion;
        // remove item from vector
        r_dens.erase(r_den_it);
        // found denominator
        found = true;
        break;
      }
      // maybe we did not find any
      // left denominator is leftover
      if (!found) l_miss_dens.push_back(l_den);
    }

    // check left-overs (ToDo: might cancel out)
    if (l_miss_nums.size() > 0 && !r_unitless) {
      throw Exception::IncompatibleUnits(n, *this);
    }
    if (l_miss_dens.size() > 0 && !r_unitless) {
      throw Exception::IncompatibleUnits(n, *this);
    }
    if (r_nums.size() > 0 && !l_unitless) {
      throw Exception::IncompatibleUnits(n, *this);
    }
    if (r_dens.size() > 0 && !l_unitless) {
      throw Exception::IncompatibleUnits(n, *this);
    }

    return factor;
  }

  // this does not cover all cases (multiple prefered units)
  bool Number::convert(const std::string& prefered, bool strict)
  {
    // no conversion if unit is empty
    if (prefered.empty()) return true;

    // first make sure same units cancel each other out
    // it seems that a map table will fit nicely to do this
    // we basically construct exponents for each unit
    // has the advantage that they will be pre-sorted
    std::map<std::string, int> exponents;

    // initialize by summing up occurences in unit vectors
    for (size_t i = 0, S = numerator_units_.size(); i < S; ++i) ++ exponents[numerator_units_[i]];
    for (size_t i = 0, S = denominator_units_.size(); i < S; ++i) -- exponents[denominator_units_[i]];

    // the final conversion factor
    double factor = 1;

    std::vector<std::string>::iterator denom_it = denominator_units_.begin();
    std::vector<std::string>::iterator denom_end = denominator_units_.end();

    // main normalization loop
    // should be close to optimal
    while (denom_it != denom_end)
    {
      // get and increment afterwards
      const std::string denom = *(denom_it ++);
      // check if conversion is needed
      if (denom == prefered) continue;
      // skip already canceled out unit
      if (exponents[denom] >= 0) continue;
      // skip all units we don't know how to convert
      if (string_to_unit(denom) == UNKNOWN) continue;
      // we now have two convertable units
      // add factor for current conversion
      factor *= conversion_factor(denom, prefered, strict);
      // update nominator/denominator exponent
      ++ exponents[denom]; -- exponents[prefered];
    }

    std::vector<std::string>::iterator nom_it = numerator_units_.begin();
    std::vector<std::string>::iterator nom_end = numerator_units_.end();

    // now search for nominator
    while (nom_it != nom_end)
    {
      // get and increment afterwards
      const std::string nom = *(nom_it ++);
      // check if conversion is needed
      if (nom == prefered) continue;
      // skip already canceled out unit
      if (exponents[nom] <= 0) continue;
      // skip all units we don't know how to convert
      if (string_to_unit(nom) == UNKNOWN) continue;
      // we now have two convertable units
      // add factor for current conversion
      factor *= conversion_factor(nom, prefered, strict);
      // update nominator/denominator exponent
      -- exponents[nom]; ++ exponents[prefered];
    }

    // now we can build up the new unit arrays
    numerator_units_.clear();
    denominator_units_.clear();

    // build them by iterating over the exponents
    for (auto exp : exponents)
    {
      // maybe there is more effecient way to push
      // the same item multiple times to a vector?
      for(size_t i = 0, S = abs(exp.second); i < S; ++i)
      {
        // opted to have these switches in the inner loop
        // makes it more readable and should not cost much
        if (!exp.first.empty()) {
          if (exp.second < 0) denominator_units_.push_back(exp.first);
          else if (exp.second > 0) numerator_units_.push_back(exp.first);
        }
      }
    }

    // apply factor to value_
    // best precision this way
    value_ *= factor;

    // success?
    return true;

  }

  // useful for making one number compatible with another
  std::string Number::find_convertible_unit() const
  {
    for (size_t i = 0, S = numerator_units_.size(); i < S; ++i) {
      std::string u(numerator_units_[i]);
      if (string_to_unit(u) != UNKNOWN) return u;
    }
    for (size_t i = 0, S = denominator_units_.size(); i < S; ++i) {
      std::string u(denominator_units_[i]);
      if (string_to_unit(u) != UNKNOWN) return u;
    }
    return std::string();
  }


  bool Number::operator== (const Expression& rhs) const
  {
    if (const Number* r = dynamic_cast<const Number*>(&rhs)) {
      size_t lhs_units = numerator_units_.size() + denominator_units_.size();
      size_t rhs_units = r->numerator_units_.size() + r->denominator_units_.size();
      // unitless and only having one unit seems equivalent (will change in future)
      if (!lhs_units || !rhs_units) {
        return std::fabs(value() - r->value()) < NUMBER_EPSILON;
      }
      return (numerator_units_ == r->numerator_units_) &&
             (denominator_units_ == r->denominator_units_) &&
             std::fabs(value() - r->value()) < NUMBER_EPSILON;
    }
    return false;
  }

  bool Number::operator< (const Number& rhs) const
  {
    size_t lhs_units = numerator_units_.size() + denominator_units_.size();
    size_t rhs_units = rhs.numerator_units_.size() + rhs.denominator_units_.size();
    // unitless and only having one unit seems equivalent (will change in future)
    if (!lhs_units || !rhs_units) {
      return value() < rhs.value();
    }

    Number tmp_r(rhs);
    tmp_r.normalize(find_convertible_unit());
    std::string l_unit(unit());
    std::string r_unit(tmp_r.unit());
    if (unit() != tmp_r.unit()) {
      error("cannot compare numbers with incompatible units", pstate());
    }
    return value() < tmp_r.value();
  }

  bool String_Quoted::operator== (const Expression& rhs) const
  {
    if (const String_Quoted* qstr = dynamic_cast<const String_Quoted*>(&rhs)) {
      return (value() == qstr->value());
    } else if (const String_Constant* cstr = dynamic_cast<const String_Constant*>(&rhs)) {
      return (value() == cstr->value());
    }
    return false;
  }

  bool String_Constant::is_invisible() const {
    return value_.empty() && quote_mark_ == 0;
  }

  bool String_Constant::operator== (const Expression& rhs) const
  {
    if (const String_Quoted* qstr = dynamic_cast<const String_Quoted*>(&rhs)) {
      return (value() == qstr->value());
    } else if (const String_Constant* cstr = dynamic_cast<const String_Constant*>(&rhs)) {
      return (value() == cstr->value());
    }
    return false;
  }

  bool String_Schema::is_left_interpolant(void) const
  {
    return length() && first()->is_left_interpolant();
  }
  bool String_Schema::is_right_interpolant(void) const
  {
    return length() && last()->is_right_interpolant();
  }

  bool String_Schema::operator== (const Expression& rhs) const
  {
    if (const String_Schema* r = dynamic_cast<const String_Schema*>(&rhs)) {
      if (length() != r->length()) return false;
      for (size_t i = 0, L = length(); i < L; ++i) {
        Expression* rv = (*r)[i];
        Expression* lv = (*this)[i];
        if (!lv || !rv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Boolean::operator== (const Expression& rhs) const
  {
    if (const Boolean* r = dynamic_cast<const Boolean*>(&rhs)) {
      return (value() == r->value());
    }
    return false;
  }

  bool Color::operator== (const Expression& rhs) const
  {
    if (const Color* r = dynamic_cast<const Color*>(&rhs)) {
      return r_ == r->r() &&
             g_ == r->g() &&
             b_ == r->b() &&
             a_ == r->a();
    }
    return false;
  }

  bool List::operator== (const Expression& rhs) const
  {
    if (const List* r = dynamic_cast<const List*>(&rhs)) {
      if (length() != r->length()) return false;
      if (separator() != r->separator()) return false;
      for (size_t i = 0, L = length(); i < L; ++i) {
        Expression* rv = (*r)[i];
        Expression* lv = (*this)[i];
        if (!lv || !rv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Map::operator== (const Expression& rhs) const
  {
    if (const Map* r = dynamic_cast<const Map*>(&rhs)) {
      if (length() != r->length()) return false;
      for (auto key : keys()) {
        Expression* lv = at(key);
        Expression* rv = r->at(key);
        if (!rv || !lv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Null::operator== (const Expression& rhs) const
  {
    return rhs.concrete_type() == NULL_VAL;
  }

  size_t List::size() const {
    if (!is_arglist_) return length();
    // arglist expects a list of arguments
    // so we need to break before keywords
    for (size_t i = 0, L = length(); i < L; ++i) {
      if (Argument* arg = dynamic_cast<Argument*>((*this)[i])) {
        if (!arg->name().empty()) return i;
      }
    }
    return length();
  }


  std::string String_Quoted::inspect() const
  {
    return quote(value_, '*');
  }

  std::string String_Constant::inspect() const
  {
    return quote(value_, '*');
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Additional method on Lists to retrieve values directly or from an encompassed Argument.
  //////////////////////////////////////////////////////////////////////////////////////////
  Expression* List::value_at_index(size_t i) {
    if (is_arglist_) {
      if (Argument* arg = dynamic_cast<Argument*>((*this)[i])) {
        return arg->value();
      } else {
        return (*this)[i];
      }
    } else {
      return (*this)[i];
    }
  }


  bool Custom_Warning::operator== (const Expression& rhs) const
  {
    if (const Custom_Warning* r = dynamic_cast<const Custom_Warning*>(&rhs)) {
      return message() == r->message();
    }
    return false;
  }

  bool Custom_Error::operator== (const Expression& rhs) const
  {
    if (const Custom_Error* r = dynamic_cast<const Custom_Error*>(&rhs)) {
      return message() == r->message();
    }
    return false;
  }


  bool Binary_Expression::is_left_interpolant(void) const
  {
    return is_interpolant() || (left() && left()->is_left_interpolant());
  }
  bool Binary_Expression::is_right_interpolant(void) const
  {
    return is_interpolant() || (right() && right()->is_right_interpolant());
  }



  std::string & str_ltrim(std::string & str)
  {
    auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( str.begin() , it2);
    return str;
  }

  std::string & str_rtrim(std::string & str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  void String_Constant::rtrim()
  {
    value_ = str_rtrim(value_);
  }
  void String_Constant::ltrim()
  {
    value_ = str_ltrim(value_);
  }
  void String_Constant::trim()
  {
    rtrim();
    ltrim();
  }

  void String_Schema::rtrim()
  {
    if (!empty()) {
      if (String* str = dynamic_cast<String*>(last())) str->rtrim();
    }
  }
  void String_Schema::ltrim()
  {
    if (!empty()) {
      if (String* str = dynamic_cast<String*>(first())) str->ltrim();
    }
  }
  void String_Schema::trim()
  {
    rtrim();
    ltrim();
  }

}