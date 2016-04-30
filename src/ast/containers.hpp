#ifndef SASS_AST_CONTAINERS_H
#define SASS_AST_CONTAINERS_H

#include <set>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <unordered_map>
#include "sass/base.h"


#include "../util.hpp"
#include "../units.hpp"
#include "../context.hpp"
#include "../position.hpp"
#include "../constants.hpp"
#include "../operation.hpp"
#include "../position.hpp"
#include "../inspect.hpp"
#include "../source_map.hpp"
#include "../environment.hpp"
#include "../error_handling.hpp"
#include "../ast_def_macros.hpp"
#include "../ast_fwd_decl.hpp"
#include "../source_map.hpp"

#include "sass.h"

#include "common.hpp"
#include "nodes.hpp"

namespace Sass {


  /////////////////////////////////////////////////////////////////////////////
  // Mixin class for AST nodes that should behave like vectors. Uses the
  // "Template Method" design pattern to allow subclasses to adjust their flags
  // when certain objects are pushed.
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class Vectorized {
    std::vector<T> elements_;
  protected:
    size_t hash_;
    void reset_hash() { hash_ = 0; }
    virtual void adjust_after_pushing(T element) { }
  public:
    Vectorized(size_t s = 0) : elements_(std::vector<T>())
    { elements_.reserve(s); }
    virtual ~Vectorized() = 0;
    size_t length() const   { return elements_.size(); }
    bool empty() const      { return elements_.empty(); }
    T last() const          { return elements_.back(); }
    T first() const         { return elements_.front(); }
    T& operator[](size_t i) { return elements_[i]; }
    virtual const T& at(size_t i) const { return elements_.at(i); }
    const T& operator[](size_t i) const { return elements_[i]; }
    Vectorized& operator<<(T element)
    {
      if (!element) return *this;
      reset_hash();
      elements_.push_back(element);
      adjust_after_pushing(element);
      return *this;
    }
    Vectorized& operator+=(Vectorized* v)
    {
      for (size_t i = 0, L = v->length(); i < L; ++i) *this << (*v)[i];
      return *this;
    }
    Vectorized& unshift(T element)
    {
      elements_.insert(elements_.begin(), element);
      return *this;
    }
    std::vector<T>& elements() { return elements_; }
    const std::vector<T>& elements() const { return elements_; }
    std::vector<T>& elements(std::vector<T>& e) { elements_ = e; return elements_; }

    virtual size_t hash()
    {
      if (hash_ == 0) {
        for (T& el : elements_) {
          hash_combine(hash_, el->hash());
        }
      }
      return hash_;
    }

    typename std::vector<T>::iterator end() { return elements_.end(); }
    typename std::vector<T>::iterator begin() { return elements_.begin(); }
    typename std::vector<T>::const_iterator end() const { return elements_.end(); }
    typename std::vector<T>::const_iterator begin() const { return elements_.begin(); }
    typename std::vector<T>::iterator erase(typename std::vector<T>::iterator el) { return elements_.erase(el); }
    typename std::vector<T>::const_iterator erase(typename std::vector<T>::const_iterator el) { return elements_.erase(el); }

  };
  template <typename T>
  inline Vectorized<T>::~Vectorized() { }


  /////////////////////////////////////////////////////////////////////////////
  // Mixin class for AST nodes that should behave like a hash table. Uses an
  // extra <std::vector> internally to maintain insertion order for interation.
  /////////////////////////////////////////////////////////////////////////////
  class Hashed {
  struct HashExpression {
    size_t operator() (Expression* ex) const {
      return ex ? ex->hash() : 0;
    }
  };
  struct CompareExpression {
    bool operator()(const Expression* lhs, const Expression* rhs) const {
      return lhs && rhs && *lhs == *rhs;
    }
  };
  typedef std::unordered_map<
    Expression*, // key
    Expression*, // value
    HashExpression, // hasher
    CompareExpression // compare
  > ExpressionMap;
  private:
    ExpressionMap elements_;
    std::vector<Expression*> list_;
  protected:
    size_t hash_;
    Expression* duplicate_key_;
    void reset_hash() { hash_ = 0; }
    void reset_duplicate_key() { duplicate_key_ = 0; }
    virtual void adjust_after_pushing(std::pair<Expression*, Expression*> p) { }
  public:
    Hashed(size_t s = 0) : elements_(ExpressionMap(s)), list_(std::vector<Expression*>())
    { elements_.reserve(s); list_.reserve(s); reset_duplicate_key(); }
    virtual ~Hashed();
    size_t length() const                  { return list_.size(); }
    bool empty() const                     { return list_.empty(); }
    bool has(Expression* k) const          { return elements_.count(k) == 1; }
    Expression* at(Expression* k) const;
    bool has_duplicate_key() const         { return duplicate_key_ != 0; }
    Expression* get_duplicate_key() const  { return duplicate_key_; }
    const ExpressionMap elements() { return elements_; }
    Hashed& operator<<(std::pair<Expression*, Expression*> p)
    {
      reset_hash();

      if (!has(p.first)) list_.push_back(p.first);
      else if (!duplicate_key_) duplicate_key_ = p.first;

      elements_[p.first] = p.second;

      adjust_after_pushing(p);
      return *this;
    }
    Hashed& operator+=(Hashed* h)
    {
      if (length() == 0) {
        this->elements_ = h->elements_;
        this->list_ = h->list_;
        return *this;
      }

      for (auto key : h->keys()) {
        *this << std::make_pair(key, h->at(key));
      }

      reset_duplicate_key();
      return *this;
    }
    const ExpressionMap& pairs() const { return elements_; }
    const std::vector<Expression*>& keys() const { return list_; }

    std::unordered_map<Expression*, Expression*>::iterator end() { return elements_.end(); }
    std::unordered_map<Expression*, Expression*>::iterator begin() { return elements_.begin(); }
    std::unordered_map<Expression*, Expression*>::const_iterator end() const { return elements_.end(); }
    std::unordered_map<Expression*, Expression*>::const_iterator begin() const { return elements_.begin(); }

  };
  inline Hashed::~Hashed() { }

}

#endif
