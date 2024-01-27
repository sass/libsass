/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VALUES_H
#define SASS_VALUES_H

#include <sass/base.h>

// Implementation Notes: While I was refactoring for LibSass 4.0, I figured we should get
// rid of all intermediate structs that we created when converting back and forth from C
// to CPP. I researched several approaches and will document my findings here. C only knows
// about structs, therefore we can export any struct-ptr from C++ directly to C. This would
// have been the most desirable approach, since any class in C++ can be turned into a struct
// but there are limitations. Mainly we cannot export a name-spaced struct, so we would need to
// define all our "classes" on the root namespace. The main benefit would be that we could
// inspect objects during debugging if linking statically. Another approach would be to wrap
// a ValueObj inside a struct. My final conclusion was to simply create an "anonymous" struct
// on the C-API side, which has no implementation at all. In the actual implementation we
// just trust the pointer to be of the type it should be, or you get undefined behavior.
// Since underlying pointers are often RefCounted, we know how to handle the reference count
// for memory management when destruction is requested from C-API. Whenever the created value
// is a e.g. added to a container, the actual destruction of the original is skipped.

// ToDo: how should we handle sass colors now that we have RGBA, HSLA and HWBA format?

#ifdef __cplusplus
extern "C" {
#endif

  // Type of Sass values
  enum SassValueType {
    SASS_BOOLEAN,
    SASS_NUMBER,
    SASS_COLOR,
    SASS_STRING,
    SASS_LIST,
    SASS_MAP,
    SASS_NULL,
    SASS_ERROR,
    SASS_WARNING,
    SASS_FUNCTION,
    SASS_CALCULATION,
    SASS_CALC_OPERATION,
    SASS_MIXIN
  };

  // List separators
  enum SassSeparator {
    SASS_COMMA,
    SASS_SPACE,
    SASS_DIV,
    // A separator that hasn't yet been determined.
    // Singleton lists and empty lists don't have separators defined. This means
    // that list functions will prefer other lists' separators if possible.
    SASS_UNDEF,
  };

  // Value Operators
  enum SassOperator {
    OR, AND,                   // logical connectives
    EQ, NEQ, GT, GTE, LT, LTE, // arithmetic relations
    ADD, SUB, MUL, DIV, MOD,   // arithmetic functions
    ASSIGN, IESEQ                      // special IE single equal
  };

  // Creator functions for all value types
  ADDAPI struct SassValue* ADDCALL sass_make_null(void);
  ADDAPI struct SassValue* ADDCALL sass_make_boolean(bool val);
  ADDAPI struct SassValue* ADDCALL sass_make_string(const char* val, bool is_quoted);
  ADDAPI struct SassValue* ADDCALL sass_make_number(double val, const char* unit);
  ADDAPI struct SassValue* ADDCALL sass_make_color(double r, double g, double b, double a);
  ADDAPI struct SassValue* ADDCALL sass_make_list(enum SassSeparator sep, bool is_bracketed);
  ADDAPI struct SassValue* ADDCALL sass_make_map(void);
  ADDAPI struct SassValue* ADDCALL sass_make_error(const char* msg);
  ADDAPI struct SassValue* ADDCALL sass_make_warning(const char* msg);

  // Generic destructor function for all types
  // Will release memory of all associated SassValue children
  // Means we will delete recursively for lists and maps
  ADDAPI void ADDCALL sass_delete_value(struct SassValue* val);

  // Make a deep cloned copy of the given sass value
  ADDAPI struct SassValue* ADDCALL sass_clone_value(struct SassValue* val);

  // Execute an operation for two Sass_Values and return the result as a Sass_Value too
  ADDAPI struct SassValue* ADDCALL sass_value_op(enum SassOperator op, struct SassValue* a, struct SassValue* b);

  // Stringify a Sass_Values and also return the result as a Sass_Value (of type STRING)
  ADDAPI struct SassValue* ADDCALL sass_value_stringify(struct SassValue* a, bool compressed, int precision);

  // Return the sass tag for a generic sass value
  // Check is needed before accessing specific values!
  ADDAPI enum SassValueType ADDCALL sass_value_get_tag(struct SassValue* v);

  // Check value to be of a specific type
  // Can also be used before accessing properties!
  ADDAPI bool ADDCALL sass_value_is_null(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_number(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_string(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_boolean(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_color(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_list(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_map(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_error(struct SassValue* v);
  ADDAPI bool ADDCALL sass_value_is_warning(struct SassValue* v);

  // Getters and setters for Sass_Number
  ADDAPI double ADDCALL sass_number_get_value(struct SassValue* v);
  ADDAPI void ADDCALL sass_number_set_value(struct SassValue* v, double value);
  ADDAPI const char* ADDCALL sass_number_get_unit(struct SassValue* v);
  ADDAPI void ADDCALL sass_number_set_unit(struct SassValue* v, const char* unit);
  ADDAPI void ADDCALL sass_number_normalize(struct SassValue* v); // What does it do?
  ADDAPI void ADDCALL sass_number_reduce(struct SassValue* v);

  // Getters and setters for Sass_String
  ADDAPI const char* ADDCALL sass_string_get_value(struct SassValue* v);
  ADDAPI void ADDCALL sass_string_set_value(struct SassValue* v, char* value);
  ADDAPI bool ADDCALL sass_string_is_quoted(struct SassValue* v);
  ADDAPI void ADDCALL sass_string_set_quoted(struct SassValue* v, bool quoted);

  // Getters and setters for Sass_Boolean
  ADDAPI bool ADDCALL sass_boolean_get_value(struct SassValue* v);
  ADDAPI void ADDCALL sass_boolean_set_value(struct SassValue* v, bool value);

  // Getters and setters for Sass_Color
  ADDAPI double ADDCALL sass_color_get_r(struct SassValue* v);
  ADDAPI void ADDCALL sass_color_set_r(struct SassValue* v, double r);
  ADDAPI double ADDCALL sass_color_get_g(struct SassValue* v);
  ADDAPI void ADDCALL sass_color_set_g(struct SassValue* v, double g);
  ADDAPI double ADDCALL sass_color_get_b(struct SassValue* v);
  ADDAPI void ADDCALL sass_color_set_b(struct SassValue* v, double b);
  ADDAPI double ADDCALL sass_color_get_a(struct SassValue* v);
  ADDAPI void ADDCALL sass_color_set_a(struct SassValue* v, double a);

  ADDAPI size_t ADDCALL sass_list_get_size(struct SassValue* list);
  ADDAPI void ADDCALL sass_list_push(struct SassValue* list, struct SassValue* value);
  ADDAPI struct SassValue* ADDCALL sass_list_at(struct SassValue* list, size_t i);
  ADDAPI struct SassValue* ADDCALL sass_list_pop(struct SassValue* list, struct SassValue* value);
  ADDAPI struct SassValue* ADDCALL sass_list_shift(struct SassValue* list, struct SassValue* value);



  // Getters and setters for Sass_List
  ADDAPI enum SassSeparator ADDCALL sass_list_get_separator(struct SassValue* v);
  ADDAPI void ADDCALL sass_list_set_separator(struct SassValue* v, enum SassSeparator separator);
  ADDAPI bool ADDCALL sass_list_get_is_bracketed(struct SassValue* v);
  ADDAPI void ADDCALL sass_list_set_is_bracketed(struct SassValue* v, bool value);
  // Getters and setters for Sass_List values
  ADDAPI struct SassValue* ADDCALL sass_list_get_value(struct SassValue* v, size_t i);
  ADDAPI void ADDCALL sass_list_set_value(struct SassValue* v, size_t i, struct SassValue* value);

  // Getter for the number of items in map
  // ADDAPI size_t ADDCALL sass_map_get_size (struct SassValue* v);

  ADDAPI void ADDCALL sass_map_set(struct SassValue* m, struct SassValue* k, struct SassValue* v);

  ADDAPI struct SassMapIterator* ADDCALL sass_map_make_iterator(struct SassValue* map);
  ADDAPI void ADDCALL sass_map_delete_iterator(struct SassMapIterator* it);
  ADDAPI bool ADDCALL sass_map_iterator_exhausted(struct SassMapIterator* it);
  ADDAPI struct SassValue* ADDCALL sass_map_iterator_get_key(struct SassMapIterator* it);
  ADDAPI struct SassValue* ADDCALL sass_map_iterator_get_value(struct SassMapIterator* it);
  ADDAPI void ADDCALL sass_map_iterator_next(struct SassMapIterator* it);

  // sass_map_get_iterator();
  // sass_map_iterator_next(it);


  ADDAPI void ADDCALL sass_map_set(struct SassValue* m, struct SassValue* k, struct SassValue* v);
  ADDAPI struct SassValue* ADDCALL sass_map_get(struct SassValue* m, struct SassValue* k);


  // Getters and setters for Sass_Map keys and values
  //ADDAPI struct SassValue* ADDCALL sass_map_get_key (struct SassValue* v, size_t i);
  //ADDAPI void ADDCALL sass_map_set_key (struct SassValue* v, size_t i, struct SassValue*);
  //ADDAPI struct SassValue* ADDCALL sass_map_get_value (struct SassValue* v, size_t i);
  //ADDAPI void ADDCALL sass_map_set_value (struct SassValue* v, size_t i, struct SassValue*);

  // Getters and setters for Sass_Error
  ADDAPI const char* ADDCALL sass_error_get_message(struct SassValue* v);
  ADDAPI void ADDCALL sass_error_set_message(struct SassValue* v, const char* msg);

  // Getters and setters for Sass_Warning
  ADDAPI const char* ADDCALL sass_warning_get_message(struct SassValue* v);
  ADDAPI void ADDCALL sass_warning_set_message(struct SassValue* v, const char* msg);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
