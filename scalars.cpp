#include "scalars.hpp"

#include <sstream>
#include <iostream>

namespace Sass {
  using namespace std;
  namespace Scalar {

    InvalidOp::InvalidOp(enum OP op, const SassValue& a, const SassValue& b)
    : runtime_error("InvalidSassOp")
    {
      stringstream ss;
      ss << a.sass_type() << " ";
      ss << op_to_string(op);
      ss << " " << b.sass_type();
      this->msg = ss.str();
    };

    const char* InvalidOp::what() const throw()
    {
      return msg.c_str();
    }

    // SOME STATIC VALUES

    static SassNull sass_null;
    static SassBool sass_true(true);
    static SassBool sass_false(false);

    // IMPLEMENT TYPE QUERIES

    const char* SassNull::sass_type() const { return "null"; }
    const char* SassBool::sass_type() const { return "bool"; }
    const char* SassNumber::sass_type() const { return "number"; }
    const char* SassString::sass_type() const { return "string"; }
    const char* SassColor::sass_type() const { return "color"; }

    const string SassNull::stringify() const { return "null"; }
    const string SassBool::stringify() const { return value ? "true" : "false"; }
    const string SassNumber::stringify() const { return "number"; }
    const string SassString::stringify() const { return value; }
    const string SassColor::stringify() const { return "color"; }

    // IMPLEMENT EQUAL

    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return SassBool(false); }
    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNull& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassBool& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassString& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassNumber& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassColor& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassNull& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassBool& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassString& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassNumber& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassColor& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }
    template < > const SassValue& sass_op < EQUAL > (const SassValue& a, const SassValue& b) { throw InvalidOp(EQUAL, a, b); return sass_false; }

    // IMPLEMENT CONCAT

    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNull& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassBool& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassString& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassNumber& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassColor& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassNull& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassBool& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassString& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassNumber& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassColor& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }
    template < > const SassValue& sass_op < CONCAT > (const SassValue& a, const SassValue& b) { throw InvalidOp(CONCAT, a, b); return sass_null; }

    // IMPLEMENT ASSIGN

    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNull& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassBool& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassString& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassNumber& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassColor& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassNull& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassBool& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassString& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassNumber& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassColor& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }
    template < > const SassValue& sass_op < ASSIGN > (const SassValue& a, const SassValue& b) { throw InvalidOp(ASSIGN, a, b); return sass_null; }

    // IMPLEMENT MODULO

    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNull& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassBool& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassString& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassNumber& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassColor& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassNull& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassBool& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassString& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassNumber& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassColor& b) { throw InvalidOp(MODULO, a, b); return sass_null; }
    template < > const SassValue& sass_op < MODULO > (const SassValue& a, const SassValue& b) { throw InvalidOp(MODULO, a, b); return sass_null; }

    // IMPLEMENT ADDITION

    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassNull& b) { return SassString("null null"); }
    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassString& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNull& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassNull& b) { return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassString& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassBool& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassNull& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassString& b) { return SassString(a.value + b.value, a.is_quoted); }
    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassString& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassNull& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassString& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassNumber& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassNull& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassString& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassColor& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassNull& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassBool& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassString& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassNumber& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassColor& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }
    template < > const SassValue& sass_op < ADDITION > (const SassValue& a, const SassValue& b) { throw InvalidOp(ADDITION, a, b); return sass_null; }

    // IMPLEMENT MULTIPLY

    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNull& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassBool& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassString& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassNumber& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassColor& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassNull& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassBool& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassString& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassNumber& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassColor& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }
    template < > const SassValue& sass_op < MULTIPLY > (const SassValue& a, const SassValue& b) { throw InvalidOp(MULTIPLY, a, b); return sass_null; }

    // IMPLEMENT SUBTRACT

    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassString& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNull& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassString& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassBool& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassString& b) { return SassString(a.value + b.value, a.is_quoted); }
    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassString& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassString& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassNumber& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassString& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassColor& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassNull& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassBool& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassString& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassNumber& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassColor& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }
    template < > const SassValue& sass_op < SUBTRACT > (const SassValue& a, const SassValue& b) { throw InvalidOp(SUBTRACT, a, b); return sass_null; }

    // IMPLEMENT DIVISION

    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNull& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassBool& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassString& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassNumber& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassColor& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassNull& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassBool& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassString& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassNumber& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassColor& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }
    template < > const SassValue& sass_op < DIVISION > (const SassValue& a, const SassValue& b) { throw InvalidOp(DIVISION, a, b); return sass_null; }

  }
}
