#include "capi_values.hpp"

#include "exceptions.hpp"
#include "ast_values.hpp"

using namespace Sass;

struct SassMapIterator {
  Hashed<ValueObj, ValueObj>::ordered_map_type::iterator pos;
  Hashed<ValueObj, ValueObj>::ordered_map_type::iterator end;
};

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Map* getMap(struct SassValue* value) { return reinterpret_cast<Map*>(value); }
  List* getList(struct SassValue* value) { return reinterpret_cast<List*>(value); }
  Value* getValue(struct SassValue* value) { return reinterpret_cast<Value*>(value); }
  Number* getNumber(struct SassValue* value) { return reinterpret_cast<Number*>(value); }
  String* getString(struct SassValue* value) { return reinterpret_cast<String*>(value); }
  Boolean* getBoolean(struct SassValue* value) { return reinterpret_cast<Boolean*>(value); }
  ColorRgba* getColor(struct SassValue* value) { return reinterpret_cast<ColorRgba*>(value); }
  CustomError* getError(struct SassValue* value) { return reinterpret_cast<CustomError*>(value); }
  CustomWarning* getWarning(struct SassValue* value) { return reinterpret_cast<CustomWarning*>(value); }

  // Return another reference to an existing value. We simply re-use the reference counted
  // object and re-implement the memory handling also partially here (SharedImpl lite).
  struct SassValue* newSassValue(Value* value) { value->refcount += 1; return value->wrap(); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return the sass tag for a generic sass value
  enum SassValueType ADDCALL sass_value_get_tag(struct SassValue* v) { return getValue(v)->getTag(); }

  // Check value for a specific type (dispatch to virtual check methods)
  bool ADDCALL sass_value_is_null(struct SassValue* val) { return Value::unwrap(val).isNull(); }
  bool ADDCALL sass_value_is_number(struct SassValue* val) { return Value::unwrap(val).isaNumber(); }
  bool ADDCALL sass_value_is_string(struct SassValue* val) { return Value::unwrap(val).isaString(); }
  bool ADDCALL sass_value_is_boolean(struct SassValue* val) { return Value::unwrap(val).isaBoolean(); }
  bool ADDCALL sass_value_is_color(struct SassValue* val) { return Value::unwrap(val).isaColor(); }
  bool ADDCALL sass_value_is_list(struct SassValue* val) { return Value::unwrap(val).isaList(); }
  bool ADDCALL sass_value_is_map(struct SassValue* val) { return Value::unwrap(val).isaMap(); }
  bool ADDCALL sass_value_is_error(struct SassValue* val) { return Value::unwrap(val).isaCustomError(); }
  bool ADDCALL sass_value_is_warning(struct SassValue* val) { return Value::unwrap(val).isaCustomWarning(); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getters and setters for Sass_Number (UB if `sass_value_is_number` is false)
  double ADDCALL sass_number_get_value(struct SassValue* number) { return getNumber(number)->value(); }
  void ADDCALL sass_number_set_value(struct SassValue* number, double value) { getNumber(number)->value(value); }
  const char* ADDCALL sass_number_get_unit(struct SassValue* number) { return getNumber(number)->unit().c_str(); }
  void ADDCALL sass_number_set_unit(struct SassValue* number, const char* unit) { getNumber(number)->unit(unit); }
  // Normalize number and its units to standard units, e.g. `ms` will become `s` (useful for comparisons)
  void ADDCALL sass_number_normalize(struct SassValue* number) { getNumber(number)->normalize(); }
  // Reduce number and its units to a minimal form, e.g. `ms*ms/s` will become `ms` (useful for output)
  void ADDCALL sass_number_reduce(struct SassValue* number) { getNumber(number)->reduce(); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getters and setters for Sass_String (UB if `sass_value_is_boolean` is false)
  const char* ADDCALL sass_string_get_value(struct SassValue* string) { return getString(string)->value().c_str(); }
  void ADDCALL sass_string_set_value(struct SassValue* string, char* value) { getString(string)->value(value); }
  bool ADDCALL sass_string_is_quoted(struct SassValue* string) { return getString(string)->hasQuotes(); }
  void ADDCALL sass_string_set_quoted(struct SassValue* string, bool quoted) { getString(string)->hasQuotes(quoted); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getters and setters for Sass_Boolean (UB if `sass_value_is_number` is false)
  bool ADDCALL sass_boolean_get_value(struct SassValue* boolean) { return getBoolean(boolean)->value(); }
  void ADDCALL sass_boolean_set_value(struct SassValue* boolean, bool value) { getBoolean(boolean)->value(value); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getters and setters for Sass_Color (UB if `sass_value_is_color` is false)
  double ADDCALL sass_color_get_r(struct SassValue* color) { return getColor(color)->r(); }
  void ADDCALL sass_color_set_r(struct SassValue* color, double r) { getColor(color)->r(r); }
  double ADDCALL sass_color_get_g(struct SassValue* color) { return getColor(color)->g(); }
  void ADDCALL sass_color_set_g(struct SassValue* color, double g) { getColor(color)->g(g); }
  double ADDCALL sass_color_get_b(struct SassValue* color) { return getColor(color)->b(); }
  void ADDCALL sass_color_set_b(struct SassValue* color, double b) { getColor(color)->b(b); }
  double ADDCALL sass_color_get_a(struct SassValue* color) { return getColor(color)->a(); }
  void ADDCALL sass_color_set_a(struct SassValue* color, double a) { getColor(color)->a(a); }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return the value stored at the given key (or nullptr if it doesn't exist)
  struct SassValue* ADDCALL sass_map_get(struct SassValue* map, struct SassValue* key)
  {
    auto value = getMap(map)->at(getValue(key));
    if (value.isNull()) return nullptr;
    return Value::wrap(value);
  }

  // Set or create the value in the map for the given key
  void ADDCALL sass_map_set(struct SassValue* map, struct SassValue* key, struct SassValue* value)
  {
    getMap(map)->insertOrSet(getValue(key), getValue(value));
  }

  // Create an iterator to loop over all key/value pairs of this map
  // This iterator will get invalid once you alter the underlying map
  struct SassMapIterator* ADDCALL sass_map_make_iterator(struct SassValue* map)
  {
    return new SassMapIterator{ getMap(map)->begin(), getMap(map)->end() };
  }

  // Delete the iterator after you are done with it
  void ADDCALL sass_map_delete_iterator(struct SassMapIterator* it)
  {
    delete it;
  }

  // Get key for the current map iterator position
  struct SassValue* ADDCALL sass_map_iterator_get_key(struct SassMapIterator* it)
  {
    return Value::wrap(it->pos->first);
  }

  // Get value for the current map iterator position
  struct SassValue* ADDCALL sass_map_iterator_get_value(struct SassMapIterator* it)
  {
    return Value::wrap(it->pos->second);
  }

  // Returns true once the iterator has reached the end
  bool ADDCALL sass_map_iterator_exhausted(struct SassMapIterator* it)
  {
    return it->pos == it->end;
  }

  // Advance the iterator to the next key/value pair or the end
  void ADDCALL sass_map_iterator_next(struct SassMapIterator* it)
  {
    it->pos += 1;
  }

  /////////////////////////////////////////////////////////////////////////
  // ToDo: should list also have an iterator, is it so useful and cool?
  // ToDo: Index access has the advantage it is not invalidated ever.
  /////////////////////////////////////////////////////////////////////////

  // Getters and setters for Sass_List
  size_t ADDCALL sass_list_get_size(struct SassValue* v) { return getList(v)->size(); }

  enum SassSeparator ADDCALL sass_list_get_separator(struct SassValue* v) { return getList(v)->separator(); }
  void ADDCALL sass_list_set_separator(struct SassValue* v, enum SassSeparator separator) { getList(v)->separator(separator); }
  bool ADDCALL sass_list_get_is_bracketed(struct SassValue* v) { return getList(v)->hasBrackets(); }
  void ADDCALL sass_list_set_is_bracketed(struct SassValue* v, bool is_bracketed) { getList(v)->hasBrackets(is_bracketed); }
  // Getters and setters for Sass_List values
  // TODO: also have at!
  struct SassValue* ADDCALL sass_list_get_value(struct SassValue* v, size_t i) { return Value::wrap(getList(v)->at(i)); } 
  void ADDCALL sass_list_set_value(struct SassValue* v, size_t i, struct SassValue* value) { getList(v)->at(i) = getValue(value); }

  // Getters and setters for Sass_Error
  const char* ADDCALL sass_error_get_message(struct SassValue* v) { return getError(v)->message().c_str(); };
  void ADDCALL sass_error_set_message(struct SassValue* v, const char* msg) { getError(v)->message(msg); };

  // Getters and setters for Sass_Warning
  const char* ADDCALL sass_warning_get_message(struct SassValue* v) { return getWarning(v)->message().c_str(); };
  void ADDCALL sass_warning_set_message(struct SassValue* v, const char* msg) { getWarning(v)->message(msg); };

  void ADDCALL sass_list_push(struct SassValue* list, struct SassValue* value) { getList(list)->append(getValue(value)); }
  void ADDCALL sass_list_unshift(struct SassValue* list, struct SassValue* value) { getList(list)->unshift(getValue(value)); }
  struct SassValue* ADDCALL sass_list_at(struct SassValue* list, size_t i) { return Value::wrap(getList(list)->at(i)); }
  struct SassValue* ADDCALL sass_list_pop(struct SassValue* list, struct SassValue* value) { return Value::wrap(getList(list)->pop()); }
  struct SassValue* ADDCALL sass_list_shift(struct SassValue* list, struct SassValue* value) { return Value::wrap(getList(list)->shift()); }

  /////////////////////////////////////////////////////////////////////////
  // Constructor functions for all value types
  /////////////////////////////////////////////////////////////////////////

  struct SassValue* ADDCALL sass_make_boolean(bool state)
  {
    return newSassValue(SASS_MEMORY_NEW(
      Boolean, SourceSpan::tmp("sass://boolean"), state));
  }

  struct SassValue* ADDCALL sass_make_number(double val, const char* unit)
  {
    return newSassValue(SASS_MEMORY_NEW(
      Number, SourceSpan::tmp("sass://number"), val, unit ? unit : ""));
  }

  struct SassValue* ADDCALL sass_make_color(double r, double g, double b, double a)
  {
    return newSassValue(SASS_MEMORY_NEW(
      ColorRgba, SourceSpan::tmp("sass://color"), r, g, b, a));
  }

  struct SassValue* ADDCALL sass_make_string(const char* val, bool is_quoted)
  {
    return newSassValue(SASS_MEMORY_NEW(
      String, SourceSpan::tmp("sass://string"), val, is_quoted));
  }

  struct SassValue* ADDCALL sass_make_list(enum SassSeparator sep, bool is_bracketed)
  {
    return newSassValue(SASS_MEMORY_NEW(
      List, SourceSpan::tmp("sass://list"), {}, sep, is_bracketed));
  }

  struct SassValue* ADDCALL sass_make_map()
  {
    return newSassValue(SASS_MEMORY_NEW(
      Map, SourceSpan::tmp("sass://map")));
  }

  struct SassValue* ADDCALL sass_make_null(void)
  {
    return newSassValue(SASS_MEMORY_NEW(
      Null, SourceSpan::tmp("sass://null")));
  }

  struct SassValue* ADDCALL sass_make_error(const char* msg)
  {
    return newSassValue(SASS_MEMORY_NEW(
      CustomError, SourceSpan::tmp("sass://error"), msg));
  }

  struct SassValue* ADDCALL sass_make_warning(const char* msg)
  {
    return newSassValue(SASS_MEMORY_NEW(
      CustomWarning, SourceSpan::tmp("sass://warning"), msg));
  }

  /////////////////////////////////////////////////////////////////////////
  // will free all associated sass values
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_delete_value(struct SassValue* val)
  {
    Value* value = getValue(val);
    if (value) {
      value->refcount -= 1;
      if (value->refcount == 0) {
        delete value;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////
  // Make a deep cloned copy of the given sass value
  /////////////////////////////////////////////////////////////////////////

  struct SassValue* ADDCALL sass_clone_value(struct SassValue* val)
  {
    Value* copy = getValue(val)->copy(SASS_MEMORY_POS_VOID);
    copy->cloneChildren(SASS_MEMORY_POS_VOID);
    return newSassValue(copy);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  struct SassValue* ADDCALL sass_value_stringify(struct SassValue* v, bool compressed, int precision)
  {
    Value* val = getValue(v);
    // Sass_Inspect_Options options(compressed ?
    //   SASS_STYLE_COMPRESSED : SASS_STYLE_NESTED, precision);
    sass::string str(val->inspect(/*options*/));
    return sass_make_string(str.c_str(), true);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // ToDo: remove and re-implement, we need to log stuff!
  struct SassValue* ADDCALL sass_value_op(enum SassOperator op, struct SassValue* left, struct SassValue* right)
  {

    Logger logger;
    SourceSpan pstate;
    Sass::ValueObj copy;
    Value* lhs = getValue(left);
    Value* rhs = getValue(right);

    try {

      switch (op) {
      case SassOperator::OR:  return Value::wrap(lhs->isTruthy() ? lhs : rhs);
      case SassOperator::AND: return Value::wrap(lhs->isTruthy() ? rhs : lhs);
      case SassOperator::ADD: copy = lhs->plus(rhs, logger, pstate); break;
      case SassOperator::SUB: copy = lhs->minus(rhs, logger, pstate); break;
      case SassOperator::MUL: copy = lhs->times(rhs, logger, pstate); break;
      case SassOperator::DIV: copy = lhs->dividedBy(rhs, logger, pstate); break;
      case SassOperator::MOD: copy = lhs->modulo(rhs, logger, pstate); break;
      case SassOperator::EQ:  return sass_make_boolean(PtrObjEqualityFn(rhs, lhs));
      case SassOperator::NEQ: return sass_make_boolean(!PtrObjEqualityFn(rhs, lhs));
      case SassOperator::GT:  return sass_make_boolean(lhs->greaterThan(rhs, logger, pstate));
      case SassOperator::GTE: return sass_make_boolean(lhs->greaterThanOrEquals(rhs, logger, pstate));
      case SassOperator::LT:  return sass_make_boolean(lhs->lessThan(rhs, logger, pstate));
      case SassOperator::LTE: return sass_make_boolean(lhs->lessThanOrEquals(rhs, logger, pstate));
      default: throw Exception::SassScriptException("Not implemented.", logger, pstate);
      }

      copy->refcount += 1;
      return copy->wrap();
    }
//
//    // simply pass the error message back to the caller for now
//    // catch (Exception::InvalidSass& e) { return sass_make_error(e.what()); }
    catch (std::bad_alloc&) { return sass_make_error("memory exhausted"); }
    catch (std::exception & e) { return sass_make_error(e.what()); }
    catch (sass::string & e) { return sass_make_error(e.c_str()); }
    catch (const char* e) { return sass_make_error(e); }
    catch (...) { return sass_make_error("unknown"); }

  }

}
