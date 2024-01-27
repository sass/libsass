#ifndef SASS_AST_CONTAINERS_H
#define SASS_AST_CONTAINERS_H

#include "hashing.hpp"
#include "ast_helpers.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Base class/container for AST nodes that should behave like vectors.
  /////////////////////////////////////////////////////////////////////////

  template <typename V>
  class Vectorized {

  protected:

    typedef SharedPtr<V> T;
    typedef Vectorized Klass;

    // The main underlying container
    sass::vector<T> elements_;

    // Hash is only calculated once and afterwards the value
    // must not be mutated, which is the case with how sass
    // works, although we must be a bit careful not to alter
    // any value that has already been added to a set or map.
    // Must create a copy if you need to alter such an object.
    mutable size_t hash_ = 0;

  public:

    // Reserve constructor
    Vectorized(size_t s = 0)
    {
      elements_.reserve(s);
    }

    // Copy constructor from other Vectorized
    Vectorized(const Vectorized<V>* vec, bool childless = false)
    {
      if (!childless) {
        hash_ = vec->hash_;
        elements_ = vec->elements_;
      }
    }

    // Copy constructor from other base vector
    Vectorized(const Vectorized<V>& vec, bool childless = false)
    {
      if (!childless) {
        hash_ = vec.hash_;
        elements_ = vec.elements_;
      }
    }

    // Copy constructor from other base vector
    Vectorized(const sass::vector<T>& vec, bool childless = false)
    {
      if (!childless) {
        elements_ = vec;
      }
    }

    // Move constructor from other base Vectorized
    Vectorized(Vectorized<T>&& vec, bool childless = false)
    {
      if (!childless) {
        hash_ = vec.hash_;
        elements_ = std::move(vec.elements_);
      }
    }

    // Move constructor from other base vector
    Vectorized(sass::vector<T>&& vec, bool childless = false)
    {
      if (!childless) {
        elements_ = std::move(vec);
      }
    }

    // Copy constructor from other base Vectorized
    Vectorized<V>& operator=(const Vectorized<V>& other)
    {
      this->hash_ = other.hash_;
      this->elements_ = other.elements_;
      return *this;
    }

    // Copy constructor from other base vector
    Vectorized<V>& operator=(const sass::vector<T>& other)
    {
      this->hash_ = 0;
      this->elements_ = other;
      return *this;
    }

    // Move constructor from other base Vectorized
    Vectorized<V>& operator=(Vectorized<V>&& other)
    {
      this->hash_ = other.hash_;
      this->elements_ = std::move(other.elements_);
      return *this;
    }

    // Move constructor from other base vector
    Vectorized<V>& operator=(sass::vector<T>&& other)
    {
      this->hash_ = 0;
      this->elements_ = std::move(other);
      return *this;
    }

    // Some simple method delegations
    void clear() { return elements_.clear(); }
    size_t size() const { return elements_.size(); }
    void reserve(size_t n) { return elements_.reserve(n); }
    bool empty() const { return elements_.empty(); }
    const T& at(size_t i) const { return elements_.at(i); }
    const T& get(size_t i) const { return elements_[i]; }
    const T& last() const { return elements_.back(); }
    const T& first() const { return elements_.front(); }

    // Setter to ensure we reset hash value
    void set(size_t i, const T& value) {
      hash_ = 0; // reset hash
      elements_.at(i) = value;
    }

    // Setter to ensure we reset hash value
    void setLast(const T& value) {
      hash_ = 0; // reset hash
      elements_.back() = value;
    }

    // Check underlying containers for equality
    bool operator== (const Vectorized<V>& rhs) const
    {
      // Abort early if sizes do not match
      if (size() != rhs.size()) return false;
      // Abort early if hashes exist and don't match
      if (hash_ && rhs.hash_ && hash_ != rhs.hash_) return false;
      // Otherwise test each node for object equality in order
      return std::equal(begin(), end(), rhs.begin(), ObjEqualityFn<T>);
    }

    // Derive unequal operator from equality check
    bool operator!= (const Vectorized<V>& rhs) const
    {
      return !(*this == rhs);
    }

    // Implicitly get the sass::vector from our object
    // Makes the Vector directly assignable to sass::vector
    // You are responsible to make a copy if needed
    // Note: since this returns the real object, we can't
    // Note: guarantee that the hash will not get out of sync
    // operator sass::vector<T>& () { hash_ = 0; return elements_; }
    // operator const sass::vector<T>& () const { return elements_; }

    // Explicitly request all elements as a real sass::vector
    // You are responsible to make a copy if needed
    // Note: since this returns the real object, we can't
    // Note: guarantee that the hash will not get out of sync
    sass::vector<T>& elements() { hash_ = 0;  return elements_; }
    const sass::vector<T>& elements() const { return elements_; }

    // Insert all items from compatible vector
    void concat(const sass::vector<T>& v)
    {
      if (v.empty()) return;
      hash_ = 0; // reset hash
      elements_.insert(end(),
        v.begin(), v.end());
    }

    // Insert all items from compatible vector
    void concat(sass::vector<T>&& v)
    {
      if (v.empty()) return;
      hash_ = 0; // reset hash
      elements_.insert(elements_.end(),
        std::make_move_iterator(v.begin()),
        std::make_move_iterator(v.end()));
    }

    // Syntactic sugar for pointers
    void concat(const Vectorized<V>* v)
    {
      if (v != nullptr) {
        return concat(v->elements_);
      }
    }

    // Insert one item on the front
    void unshift(const T& element)
    {
      hash_ = 0; // reset hash
      elements_.insert(begin(),
        std::copy(element));
    }

    // Insert one item on the front
    void unshift(T&& element)
    {
      hash_ = 0; // reset hash
      elements_.insert(begin(),
        std::move(element));
    }

    // Remove and return item on the front
    T shift() {
      T head = first();
      hash_ = 0; // reset hash
      elements_.erase(begin());
      return head;
    }

    // Remove and return item on the back
    T pop() {
      T tail = last();
      hash_ = 0; // reset hash
      elements_.pop_back();
      return tail;
    }

    // Insert one item on the back
    // ToDo: rename this to push?
    void append(const T& element)
    {
      hash_ = 0; // reset hash
      elements_.emplace_back(element);
    }

    // Insert one item on the back
    // ToDo: rename this to push?
    void append(T&& element)
    {
      hash_ = 0; // reset hash
      elements_.emplace_back(std::move(element));
    }

    // Check if an item already exists
    // Uses underlying object `operator==`
    // E.g. compares the actual objects
    bool contains(const T& el) const
    {
      for (const T& rhs : elements_) {
        // Test the underlying objects for equality
        // A std::find checks for pointer equality
        if (ObjEqualityFn(el, rhs)) {
          return true;
        }
      }
      return false;
    }

    // Check if an item already exists
    // Uses underlying object `operator==`
    // E.g. compares the actual objects
    bool contains(const V* el) const
    {
      for (const T& rhs : elements_) {
        // Test the underlying objects for equality
        // A std::find checks for pointer equality
        if (PtrObjEqualityFn(el, rhs.ptr())) {
          return true;
        }
      }
      return false;
    }

    template <typename P>
    typename sass::vector<T>::iterator insert(P position, const T& val) {
      return elements_.insert(position, val);
    }

    template <typename P>
    typename sass::vector<T>::iterator insert(P position, T&& val) {
      return elements_.insert(position, std::move(val));
    }

    template<class UnaryPredicate>
    void eraseIf(UnaryPredicate* predicate)
    {
      elements_.erase(
        std::remove_if(
          elements_.begin(),
          elements_.end(),
          predicate),
        elements_.end());
    }

    size_t hash() const
    {
      if (hash_ == 0) {
        hash_start(hash_, typeid(Vectorized<V>).hash_code());
        for (auto child : elements_) {
          hash_combine(hash_, child->hash());
        }
      }
      return hash_;
    }

    typename sass::vector<T>::iterator end() { return elements_.end(); }
    typename sass::vector<T>::iterator begin() { return elements_.begin(); }
    typename sass::vector<T>::const_iterator end() const { return elements_.end(); }
    typename sass::vector<T>::const_iterator begin() const { return elements_.begin(); }
    typename sass::vector<T>::iterator erase(typename sass::vector<T>::iterator el) { return elements_.erase(el); }
    typename sass::vector<T>::const_iterator erase(typename sass::vector<T>::const_iterator el) { return elements_.erase(el); }

  };

  /////////////////////////////////////////////////////////////////////////
  // Mixin class for AST nodes that should behave like a hash table. Uses an
  // extra <std::vector> internally to maintain insertion order for iteration.
  /////////////////////////////////////////////////////////////////////////

  template <typename K, typename T>
  class Hashed {

  public:

    using ordered_map_type = typename OrderedMap<
      K, T, ObjHash, ObjEquality,
      Sass::Allocator<std::pair<K, T>>,
      sass::vector<std::pair<K, T>>
    >;

  protected:

    // The main underlying container
    ordered_map_type elements_;

    // Hash is only calculated once and afterwards the value
    // must not be mutated, which is the case with how sass
    // works, although we must be a bit careful not to alter
    // any value that has already been added to a set or map.
    // Must create a copy if you need to alter such an object.
    mutable size_t hash_;

  public:

    Hashed() :
      elements_(),
      hash_(0)
    {}

    // Copy constructor
    Hashed(const Hashed<K, T>& copy) :
      elements_(), hash_(0)
    {
      elements_ = copy.elements_;
    };

    // Move constructor
    Hashed(Hashed<K, T>&& move) noexcept :
      elements_(std::move(move.elements_)),
      hash_(move.hash_) {};

    // Move constructor
    Hashed(ordered_map_type&& values) :
      elements_(std::move(values)),
      hash_(0) {};

    size_t size() const { return elements_.size(); }
    bool empty() const { return elements_.empty(); }

    bool has(const K& k) const {
      return elements_.count(k) == 1;
    }

    T at(const K& k) const {
      auto it = elements_.find(k);
      if (it == elements_.end()) return {};
      else return it->second;
    }

    bool erase(const K& key)
    {
      // Would be faster by quite a bit (2% for bolt)
      // return elements_.unordered_erase(key) != 0;
      return elements_.erase(key) != 0;
    }

    typename ordered_map_type::const_iterator find(const K& key) const
    {
      return elements_.find(key);
    }

    typename ordered_map_type::iterator find(const K& key)
    {
      return elements_.find(key);
    }

    void insert(std::pair<K, T>&& kv)
    {
      elements_.insert(kv);
    }

    void insert(const std::pair<K, T>& kv)
    {
      elements_.insert(kv);
    }

    void insert(const K& key, const T& val)
    {
      insert(std::make_pair(key, val));
    }

    void insertOrSet(std::pair<K, T>& kv)
    {
      auto exists = elements_.find(kv.first);
      if (exists == elements_.end()) {
        // Insert a new entry
        elements_.insert(kv);
      }
      else {
        // Update existing entry
        exists.value() = kv.second;
      }
    }

    void insertOrSet(const K& key, const T& val)
    {
      elements_[key] = val;
    }

    void insertOrSet(const K& key, T&& val)
    {
      elements_[key] = std::move(val);
    }

    // Return modifiable reference
    ordered_map_type& elements() {
      return elements_;
    }

    sass::vector<K> keys() const {
      sass::vector<T> list;
      for (auto kv : elements_) {
        list.emplace_back(kv.first);
      }
      return list;
    }
    sass::vector<T> values() const {
      sass::vector<T> list;
      for (auto kv : elements_) {
        list.emplace_back(kv.second);
      }
      return list;
    }

    size_t hash() const
    {
      if (hash_ == 0) {
        hash_start(hash_, typeid(this).hash_code());
        for (auto kv : elements_) {
          hash_combine(hash_, kv.first->hash());
          hash_combine(hash_, kv.second->hash());
        }
      }
      return hash_;
    }

    typename ordered_map_type::iterator end() { return elements_.end(); }
    typename ordered_map_type::iterator begin() { return elements_.begin(); }
    typename ordered_map_type::const_iterator end() const { return elements_.end(); }
    typename ordered_map_type::const_iterator begin() const { return elements_.begin(); }

  };

};

#endif
