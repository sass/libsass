## LibSass C-API for sass values

`SassValue` is the base type to exchange sass values between implementors and
LibSass. Sass knows various different value types (including nested arrays
and hash-maps). If you implement a binding to another programming language, you
have to find a way to [marshal][1] (convert) a `SassValue` between the target
language and C. `SassValue` is currently only used by custom functions, but
it should also be able to use them without any explicit compiler.

[1]: https://en.wikipedia.org/wiki/Marshalling_%28computer_science%29

### Handling of containers

There are two `SassValue` types (list and map) that act as containers for
nested `SassValue` objects. They have different implementations to iterate
over the existing values. The `SassList` acts like any regular array, so you
can get the size and access every item by offset. The `SassMap` has a specific
iterator you need to allocate first in order to iterate over it via `next`.
You also have to make sure to release the memory associated with the iterator.

#### Iterating over `SassList`

```C
for (int i = 0; i < sass_list_get_size(list); i += 1) {
  struct SassValue* child = sass_list_at(list, i);
}
```

#### Iterating over `SassMap`

```C
struct SassMapIterator* it sass_map_make_iterator(map);
while (!sass_map_iterator_exhausted(it)) {
  struct SassValue* key = sass_map_iterator_get_key(it);
  struct SassValue* val = sass_map_iterator_get_value(it);
  sass_map_iterator_next(it);
}
sass_map_delete_iterator(it);
```

### Errors and warnings

Custom functions may fail for any reason and in order to communicate this state
back to LibSass, any custom function can return a `SassError`, which is a special
type of `SassValue`, solely existing for this purpose. If a custom function returns
this special type, it will throw an error further down.

Note: `SassWarning` is currently handled in the same way, but warning should ultimately
be a C-API function on its own, as we might want to emit multiple warnings, but still
return a successful return state (we only can have one error, but many warnings).

### Example code

See [sass value code example](api-value-example.md).

### Basic Usage

```C
#include <sass/values.h>
```

```C
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
  SASS_FUNCTION
};

// List separators
enum SassSeparator {
  SASS_COMMA,
  SASS_SPACE,
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
  IESEQ                      // special IE single equal
};
```

### Sass Value API

```C
// Creator functions for all value types
struct SassValue* sass_make_null(void);
struct SassValue* sass_make_boolean(bool val);
struct SassValue* sass_make_string(const char* val, bool is_quoted);
struct SassValue* sass_make_number(double val, const char* unit);
struct SassValue* sass_make_color(double r, double g, double b, double a);
struct SassValue* sass_make_list(enum SassSeparator sep, bool is_bracketed);
struct SassValue* sass_make_map(void);
struct SassValue* sass_make_error(const char* msg);
struct SassValue* sass_make_warning(const char* msg);

// Generic destructor function for all types
// Will release memory of all associated SassValue children
// Means we will delete recursively for lists and maps
void sass_delete_value(struct SassValue* val);

// Make a deep cloned copy of the given sass value
struct SassValue* sass_clone_value(struct SassValue* val);

// Execute an operation for two Sass_Values and return the result as a Sass_Value too
struct SassValue* sass_value_op(enum SassOperator op, struct SassValue* a, struct SassValue* b);

// Stringify a Sass_Values and also return the result as a Sass_Value (of type STRING)
struct SassValue* sass_value_stringify(struct SassValue* a, bool compressed, int precision);

// Return the sass tag for a generic sass value
// Check is needed before accessing specific values!
enum SassValueType sass_value_get_tag(struct SassValue* v);

// Check value to be of a specific type
// Can also be used before accessing properties!
bool sass_value_is_null(struct SassValue* v);
bool sass_value_is_number(struct SassValue* v);
bool sass_value_is_string(struct SassValue* v);
bool sass_value_is_boolean(struct SassValue* v);
bool sass_value_is_color(struct SassValue* v);
bool sass_value_is_list(struct SassValue* v);
bool sass_value_is_map(struct SassValue* v);
bool sass_value_is_error(struct SassValue* v);
bool sass_value_is_warning(struct SassValue* v);

// Getters and setters for Sass_Number
double sass_number_get_value(struct SassValue* v);
void sass_number_set_value(struct SassValue* v, double value);
const char* sass_number_get_unit(struct SassValue* v);
void sass_number_set_unit(struct SassValue* v, const char* unit);
void sass_number_normalize(struct SassValue* v); // What does it do?
void sass_number_reduce(struct SassValue* v);

// Getters and setters for Sass_String
const char* sass_string_get_value(struct SassValue* v);
void sass_string_set_value(struct SassValue* v, char* value);
bool sass_string_is_quoted(struct SassValue* v);
void sass_string_set_quoted(struct SassValue* v, bool quoted);

// Getters and setters for Sass_Boolean
bool sass_boolean_get_value(struct SassValue* v);
void sass_boolean_set_value(struct SassValue* v, bool value);

// Getters and setters for Sass_Color
double sass_color_get_r(struct SassValue* v);
void sass_color_set_r(struct SassValue* v, double r);
double sass_color_get_g(struct SassValue* v);
void sass_color_set_g(struct SassValue* v, double g);
double sass_color_get_b(struct SassValue* v);
void sass_color_set_b(struct SassValue* v, double b);
double sass_color_get_a(struct SassValue* v);
void sass_color_set_a(struct SassValue* v, double a);

size_t sass_list_get_size(struct SassValue* list);
void sass_list_push(struct SassValue* list, struct SassValue* value);
struct SassValue* sass_list_at(struct SassValue* list, size_t i);
struct SassValue* sass_list_pop(struct SassValue* list, struct SassValue* value);
struct SassValue* sass_list_shift(struct SassValue* list, struct SassValue* value);

// Getters and setters for Sass_List
enum SassSeparator sass_list_get_separator(struct SassValue* v);
void sass_list_set_separator(struct SassValue* v, enum SassSeparator separator);
bool sass_list_get_is_bracketed(struct SassValue* v);
void sass_list_set_is_bracketed(struct SassValue* v, bool value);
// Getters and setters for Sass_List values
struct SassValue* sass_list_get_value(struct SassValue* v, size_t i);
void sass_list_set_value(struct SassValue* v, size_t i, struct SassValue* value);

void sass_map_set(struct SassValue* m, struct SassValue* k, struct SassValue* v);
struct SassMapIterator* sass_map_make_iterator(struct SassValue* map);
void sass_map_delete_iterator(struct SassMapIterator* it);
bool sass_map_iterator_exhausted(struct SassMapIterator* it);
struct SassValue* sass_map_iterator_get_key(struct SassMapIterator* it);
struct SassValue* sass_map_iterator_get_value(struct SassMapIterator* it);
void sass_map_iterator_next(struct SassMapIterator* it);

void sass_map_set(struct SassValue* m, struct SassValue* k, struct SassValue* v);
struct SassValue* sass_map_get(struct SassValue* m, struct SassValue* k);

// Getters and setters for Sass_Error
const char* sass_error_get_message(struct SassValue* v);
void sass_error_set_message(struct SassValue* v, const char* msg);

// Getters and setters for Sass_Warning
const char* sass_warning_get_message(struct SassValue* v);
void sass_warning_set_message(struct SassValue* v, const char* msg);
```

### More links

- [Sass Value Example](api-value-example.md)

