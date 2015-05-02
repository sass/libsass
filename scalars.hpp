#ifndef SASS_SCALARS_H
#define SASS_SCALARS_H

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

// This is highly familiar with how perl handles data structures.
// There are certain basic data structures and native data types.
// Basic data structures include arrays, hashes (maps) and scalars
// Scalars are specific data types which attach certain semantics.

// Perl also has a reference data types (which is just another
// native scalar type) to store references to other data stores.
// So a reference can point to arrays, hashes (maps) and scalars.

namespace Sass {
  using namespace std;
  namespace Scalar {

    enum OP {
      EQUAL,
      CONCAT,
      MODULO,
      ASSIGN,
      ADDITION,
      SUBTRACT,
      MULTIPLY,
      DIVISION
    };

    inline const char* op_to_string(enum OP op)
    {
      switch (op)
      {
        case EQUAL: return "eq"; break;
        case CONCAT: return "cat"; break;
        case MODULO: return "mod"; break;
        case ASSIGN: return "assign"; break;
        case ADDITION: return "plus"; break;
        case SUBTRACT: return "minus"; break;
        case MULTIPLY: return "times"; break;
        case DIVISION: return "div"; break;
      }
      return 0;
    }

    class SassNull;
    class SassBool;
    class SassString;
    class SassNumber;
    class SassColor;
    class SassValue;
    template <typename>
    class SassValue_CRTP;

    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassValue& a, const SassValue& b);

    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassNull& a, const SassValue& b);

    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassBool& a, const SassValue& b);

    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassString& a, const SassValue& b);

    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassNumber& a, const SassValue& b);

    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassNull& b);
    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassBool& b);
    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassString& b);
    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassNumber& b);
    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassColor& b);
    template < enum OP T > const SassValue& sass_op(const SassColor& a, const SassValue& b);

    // custom error (always use try catch for operations)
    class InvalidOp : public runtime_error
    {
      public:
        string msg;
        virtual const char* what() const throw();
        InvalidOp(enum OP op, const SassValue& a, const SassValue& b);
    };

    // abstract base class
    class SassValue
    {
      friend class SassValue_CRTP < SassNull >;
      friend class SassValue_CRTP < SassBool >;
      friend class SassValue_CRTP < SassNumber >;
      friend class SassValue_CRTP < SassString >;
      friend class SassValue_CRTP < SassColor >;
      public:
        virtual ~SassValue() {};
        virtual const SassValue& clone() const = 0;
        virtual const char* sass_type() const = 0;
        virtual const string stringify() const = 0;
        virtual void operator=(const SassValue& value) = 0;
        virtual const SassValue& operator+(const SassValue& value) const = 0;
      protected:
        // resolve right hand value also via virtual table call
        // throw error if we have no specific implementation
        virtual const SassValue& op_equal(const SassValue& value) const
        { throw InvalidOp(EQUAL, value, *this); }
        virtual const SassValue& op_concat(const SassValue& value) const
        { throw InvalidOp(CONCAT, value, *this); }
        virtual const SassValue& op_modulo(const SassValue& value) const
        { throw InvalidOp(MODULO, value, *this); }
        virtual const SassValue& op_addition(const SassValue& value) const
        { throw InvalidOp(ADDITION, value, *this); }
        virtual const SassValue& op_subtract(const SassValue& value) const
        { throw InvalidOp(SUBTRACT, value, *this); }
        virtual const SassValue& op_multiply(const SassValue& value) const
        { throw InvalidOp(MULTIPLY, value, *this); }
        virtual const SassValue& op_division(const SassValue& value) const
        { throw InvalidOp(DIVISION, value, *this); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_equal(const SassNull& value) const = 0;
        virtual const SassValue& op_equal(const SassBool& value) const = 0;
        virtual const SassValue& op_equal(const SassString& value) const = 0;
        virtual const SassValue& op_equal(const SassNumber& value) const = 0;
        virtual const SassValue& op_equal(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_concat(const SassNull& value) const = 0;
        virtual const SassValue& op_concat(const SassBool& value) const = 0;
        virtual const SassValue& op_concat(const SassString& value) const = 0;
        virtual const SassValue& op_concat(const SassNumber& value) const = 0;
        virtual const SassValue& op_concat(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_assign(const SassNull& value) const = 0;
        virtual const SassValue& op_assign(const SassBool& value) const = 0;
        virtual const SassValue& op_assign(const SassString& value) const = 0;
        virtual const SassValue& op_assign(const SassNumber& value) const = 0;
        virtual const SassValue& op_assign(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_modulo(const SassNull& value) const = 0;
        virtual const SassValue& op_modulo(const SassBool& value) const = 0;
        virtual const SassValue& op_modulo(const SassString& value) const = 0;
        virtual const SassValue& op_modulo(const SassNumber& value) const = 0;
        virtual const SassValue& op_modulo(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_addition(const SassNull& value) const = 0;
        virtual const SassValue& op_addition(const SassBool& value) const = 0;
        virtual const SassValue& op_addition(const SassString& value) const = 0;
        virtual const SassValue& op_addition(const SassNumber& value) const = 0;
        virtual const SassValue& op_addition(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_subtract(const SassNull& value) const = 0;
        virtual const SassValue& op_subtract(const SassBool& value) const = 0;
        virtual const SassValue& op_subtract(const SassString& value) const = 0;
        virtual const SassValue& op_subtract(const SassNumber& value) const = 0;
        virtual const SassValue& op_subtract(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_multiply(const SassNull& value) const = 0;
        virtual const SassValue& op_multiply(const SassBool& value) const = 0;
        virtual const SassValue& op_multiply(const SassString& value) const = 0;
        virtual const SassValue& op_multiply(const SassNumber& value) const = 0;
        virtual const SassValue& op_multiply(const SassColor& value) const = 0;
        // dispatch operations to the specific implementations
        virtual const SassValue& op_division(const SassNull& value) const = 0;
        virtual const SassValue& op_division(const SassBool& value) const = 0;
        virtual const SassValue& op_division(const SassString& value) const = 0;
        virtual const SassValue& op_division(const SassNumber& value) const = 0;
        virtual const SassValue& op_division(const SassColor& value) const = 0;
    };

    template <typename Derived>
    class SassValue_CRTP : public SassValue
    {
      public:
        virtual const SassValue& clone() const
        { return *(new Derived(static_cast<Derived const&>(*this))); }

        virtual void operator=(const SassValue& value) {
        	cerr << "assign op\n";
        	 }
        // resolve right hand value also via virtual table call
        // throw error if we have no specific implementation
        // virtual void operator=(const SassValue& value) const
        // { return value.op_assign(static_cast<Derived const&>(*this)); }
        virtual const SassValue& operator%(const SassValue& value) const
        { return value.op_modulo(static_cast<Derived const&>(*this)); }
        virtual const SassValue& operator+(const SassValue& value) const
        { return value.op_addition(static_cast<Derived const&>(*this)); }
        virtual const SassValue& operator-(const SassValue& value) const
        { return value.op_subtract(static_cast<Derived const&>(*this)); }
        virtual const SassValue& operator*(const SassValue& value) const
        { return value.op_multiply(static_cast<Derived const&>(*this)); }
        virtual const SassValue& operator/(const SassValue& value) const
        { return value.op_division(static_cast<Derived const&>(*this)); }
      protected:
        // dispatch operations to the specific implementations
        virtual const SassValue& op_equal(const SassNull& value) const
        { return sass_op < EQUAL >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_equal(const SassBool& value) const
        { return sass_op < EQUAL >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_equal(const SassString& value) const
        { return sass_op < EQUAL >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_equal(const SassNumber& value) const
        { return sass_op < EQUAL >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_equal(const SassColor& value) const
        { return sass_op < EQUAL >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_concat(const SassNull& value) const
        { return sass_op < CONCAT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_concat(const SassBool& value) const
        { return sass_op < CONCAT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_concat(const SassString& value) const
        { return sass_op < CONCAT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_concat(const SassNumber& value) const
        { return sass_op < CONCAT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_concat(const SassColor& value) const
        { return sass_op < CONCAT >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_assign(const SassNull& value) const
        { return sass_op < ASSIGN >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_assign(const SassBool& value) const
        { return sass_op < ASSIGN >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_assign(const SassString& value) const
        { return sass_op < ASSIGN >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_assign(const SassNumber& value) const
        { return sass_op < ASSIGN >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_assign(const SassColor& value) const
        { return sass_op < ASSIGN >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_modulo(const SassNull& value) const
        { return sass_op < MODULO >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_modulo(const SassBool& value) const
        { return sass_op < MODULO >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_modulo(const SassString& value) const
        { return sass_op < MODULO >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_modulo(const SassNumber& value) const
        { return sass_op < MODULO >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_modulo(const SassColor& value) const
        { return sass_op < MODULO >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_addition(const SassNull& value) const
        { return sass_op < ADDITION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_addition(const SassBool& value) const
        { return sass_op < ADDITION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_addition(const SassString& value) const
        { return sass_op < ADDITION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_addition(const SassNumber& value) const
        { return sass_op < ADDITION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_addition(const SassColor& value) const
        { return sass_op < ADDITION >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_subtract(const SassNull& value) const
        { return sass_op < SUBTRACT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_subtract(const SassBool& value) const
        { return sass_op < SUBTRACT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_subtract(const SassString& value) const
        { return sass_op < SUBTRACT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_subtract(const SassNumber& value) const
        { return sass_op < SUBTRACT >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_subtract(const SassColor& value) const
        { return sass_op < SUBTRACT >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_multiply(const SassNull& value) const
        { return sass_op < MULTIPLY >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_multiply(const SassBool& value) const
        { return sass_op < MULTIPLY >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_multiply(const SassString& value) const
        { return sass_op < MULTIPLY >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_multiply(const SassNumber& value) const
        { return sass_op < MULTIPLY >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_multiply(const SassColor& value) const
        { return sass_op < MULTIPLY >(value, static_cast<Derived const&>(*this)); }
        // dispatch operations to the specific implementations
        virtual const SassValue& op_division(const SassNull& value) const
        { return sass_op < DIVISION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_division(const SassBool& value) const
        { return sass_op < DIVISION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_division(const SassString& value) const
        { return sass_op < DIVISION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_division(const SassNumber& value) const
        { return sass_op < DIVISION >(value, static_cast<Derived const&>(*this)); }
        virtual const SassValue& op_division(const SassColor& value) const
        { return sass_op < DIVISION >(value, static_cast<Derived const&>(*this)); }
    };

    class SassNull
    : public SassValue_CRTP
               < SassNull >
    {
      public:
        SassNull() {
        	cerr << "ctor null\n";
        };
        virtual const char* sass_type() const;
        virtual const string stringify() const;
        // virtual SassNull& operator=(const SassNull& value);
        // virtual SassNull& operator=(const SassValue& value);
        virtual ~SassNull() {
        	cerr << "dtor null\n";
        };
    };


    class SassBool
    : public SassValue_CRTP
               < SassBool >
    {
      private:
        bool value;
      public:
        SassBool(bool value)
        {
        	cerr << "ctor bool\n";
          this->value = value;
        }
        virtual const char* sass_type() const;
        virtual const string stringify() const;
        /*
        SassBool operator=(SassBool value) {
        	cout << "assign op\n";
        	return SassBool(false); }
        SassBool operator=(const SassBool& value) {
        	cout << "assign op\n";
        	return SassBool(false); }
        */

        // virtual SassBool& operator=(const SassBool& value);
        // virtual SassBool& operator=(const SassValue& value);
        virtual ~SassBool() {
        	cerr << "dtor bool\n";
        	};
    };

    class SassNumber
    : public SassValue_CRTP
               < SassNumber >
    {
      private:
        double value;
      public:
        SassNumber(double value, const string& unit = "")
        {
          this->value = value;
        }
        virtual const char* sass_type() const;
        virtual const string stringify() const;
        // virtual SassNumber& operator=(const SassNumber& value);
        // virtual SassNumber& operator=(const SassValue& value);
        virtual ~SassNumber() {};
    };

    class SassString
    : public SassValue_CRTP
               < SassString >
    {
      public:
        string value;
        bool is_quoted;
      public:

        SassString(const SassString& rhs)
        {
        }

        SassString(string value, bool is_quoted = false)
        {
        	cerr << "ctor string\n";
          this->value = value;
          this->is_quoted = is_quoted;
        }
/*
        virtual SassString operator=(SassString value) {
        	cout << "assign copy op\n";
        	return SassString("test"); }
*/
/*
        virtual const SassString& operator=(const SassValue& other) {
          return *this;
        }

        virtual const SassString& operator=(const SassString& other) {
        	cout << "assign move op\n";
          SassString tmp(other);
          std::swap(this->value, tmp.value);
          std::swap(this->is_quoted, tmp.is_quoted);
        	return *this; }
*/

        virtual const char* sass_type() const;
        virtual const string stringify() const;
        // virtual SassString& operator=(const SassString& value);
        // virtual SassString& operator=(const SassValue& value);
        virtual ~SassString() {
        	cerr << "dtor string\n";
        };
    };

    class SassColor
    : public SassValue_CRTP
               < SassColor >
    {
      private:
        double r;
        double g;
        double b;
        double a;
      public:
        SassColor(double r, double g, double b, double a = 1.0)
        {
          this->r = r;
          this->g = g;
          this->b = b;
          this->a = a;
        }
        virtual const char* sass_type() const;
        virtual const string stringify() const;
        // virtual SassColor& operator=(const SassColor& value);
        // virtual SassColor& operator=(const SassValue& value);
        virtual ~SassColor() {};
    };

  }
}

#endif
