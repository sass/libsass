#ifndef SASS_AST_HELPERS_H
#define SASS_AST_HELPERS_H

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"
#include <algorithm>

namespace Sass {

  // ###########################################################################
  // ###########################################################################

  // easier to search with name
  const bool DELAYED = true;

  // ToDo: should this really be hardcoded
  // Note: most methods follow precision option
  const double NUMBER_EPSILON = 1e-12;

  // macro to test if numbers are equal within a small error margin
  #define NEAR_EQUAL(lhs, rhs) std::fabs(lhs - rhs) < NUMBER_EPSILON

  // ###########################################################################
  // We define various functions and functors here.
  // Functions satisfy the BinaryPredicate requirement
  // Functors are structs used for e.g. unordered_map
  // ###########################################################################

  // ###########################################################################
  // Implement compare and hashing operations for raw pointers
  // ###########################################################################

  template <class T>
  size_t PtrHashFn(const T* ptr) {
    return std::hash<std::size_t>()((size_t)ptr);
  }

  struct PtrHash {
    template <class T>
    size_t operator() (const T* ptr) const {
      return PtrHashFn(ptr);
    }
  };

  template <class T>
  bool PtrEqualityFn(const T* lhs, const T* rhs) {
    if (lhs == nullptr) return rhs == nullptr;
    if (rhs == nullptr) return lhs == nullptr;
    return lhs == rhs; // compare raw pointers
  }

  struct PtrEquality {
    template <class T>
    bool operator() (const T* lhs, const T* rhs) const {
      return PtrEqualityFn<T>(lhs, rhs);
    }
  };

  // ###########################################################################
  // Implement compare and hashing operations for AST Nodes
  // ###########################################################################

  // TODO: get rid of funtions and use ObjEquality<T>

  template <class T>
  // Hash the raw pointer instead of object
  size_t ObjPtrHashFn(const T& obj) {
    return PtrHashFn(obj.ptr());
  }

  struct ObjPtrHash {
    template <class T>
    // Hash the raw pointer instead of object
    size_t operator() (const T& obj) const {
      return ObjPtrHashFn(obj);
    }
  };

  template <class T>
  // Hash the object and its content
  size_t ObjHashFn(const T& obj) {
    return obj ? obj->hash() : 0;
  }

  struct ObjHash {
    template <class T>
    // Hash the object and its content
    size_t operator() (const T& obj) const {
      return ObjHashFn(obj);
    }
  };

  template <class T>
  // Hash the object behind pointer
  size_t PtrObjHashFn(const T* obj) {
    return obj ? obj->hash() : 0;
  }

  struct PtrObjHash {
    template <class T>
    // Hash the object behind pointer
    size_t operator() (const T* obj) const {
      return PtrObjHashFn(obj);
    }
  };

  template <class T>
  // Compare raw pointers to the object
  bool ObjPtrEqualityFn(const T& lhs, const T& rhs) {
    return PtrEqualityFn(lhs.ptr(), rhs.ptr());
  }

  struct ObjPtrEquality {
    template <class T>
    // Compare raw pointers to the object
    bool operator() (const T& lhs, const T& rhs) const {
      return ObjPtrEqualityFn<T>(lhs, rhs);
    }
  };

  template <class T>
  // Compare the objects behind the pointers
  bool PtrObjEqualityFn(const T* lhs, const T* rhs) {
    if (lhs == nullptr) return rhs == nullptr;
    else if (rhs == nullptr) return false;
    else return *lhs == *rhs;
  }

  struct PtrObjEquality {
    template <class T>
    // Compare the objects behind the pointers
    bool operator() (const T* lhs, const T* rhs) const {
      return PtrObjEqualityFn<T>(lhs, rhs);
    }
  };

  template <class T>
  // Compare the objects and its contents
  bool ObjEqualityFn(const T& lhs, const T& rhs) {
    return PtrObjEqualityFn(lhs.ptr(), rhs.ptr());
  }

  struct ObjEquality {
    template <class T>
    // Compare the objects and its contents
    bool operator() (const T& lhs, const T& rhs) const {
      return ObjEqualityFn<T>(lhs, rhs);
    }
  };

  // ###########################################################################
  // Implement ordering operations for AST Nodes
  // ###########################################################################

  template <class T>
  // Compare the objects behind pointers
  bool PtrObjLessThanFn(const T* lhs, const T* rhs) {
    if (lhs == nullptr) return rhs != nullptr;
    else if (rhs == nullptr) return false;
    else return *lhs < *rhs;
  }

  struct PtrObjLessThan {
    template <class T>
    // Compare the objects behind pointers
    bool operator() (const T* lhs, const T* rhs) const {
      return PtrObjLessThanFn<T>(lhs, rhs);
    }
  };

  template <class T>
  // Compare the objects and its content
  bool ObjLessThanFn(const T& lhs, const T& rhs) {
    return PtrObjLessThanFn(lhs.ptr(), rhs.ptr());
  };

  struct ObjLessThan {
    template <class T>
    // Compare the objects and its content
    bool operator() (const T& lhs, const T& rhs) const {
      return ObjLessThanFn<T>(lhs, rhs);
    }
  };

  // ###########################################################################
  // Some STL helper functions
  // ###########################################################################

  // Check if all elements are equal
  template <class X, class Y,
    typename XT = typename X::value_type,
    typename YT = typename Y::value_type>
  bool ListEquality(const X& lhs, const Y& rhs,
    bool(*cmp)(const XT*, const YT*))
  {
    return lhs.size() == rhs.size() &&
      std::equal(lhs.begin(), lhs.end(),
        rhs.begin(), cmp);
  }

  // Return if Vector is empty
  template <class T>
  bool listIsEmpty(T* cnt) {
    return cnt && cnt->empty();
  }

  // Erase items from vector that match predicate
  template<class T, class UnaryPredicate>
  void listEraseItemIf(T& vec, UnaryPredicate* predicate)
  {
    vec.erase(std::remove_if(vec.begin(), vec.end(), predicate), vec.end());
  }

  // Check that every item in `lhs` is also in `rhs`
  // Note: this works by comparing the raw pointers
  template <typename T>
  bool listIsSubsetOrEqual(const T& lhs, const T& rhs) {
    for (auto const& item : lhs) {
      if (std::find(rhs.begin(), rhs.end(), item) == rhs.end())
        return false;
    }
    return true;
  }

  // ##########################################################################
  // Special case insensitive string matcher. We can optimize
  // the more general compare case quite a bit by requiring
  // consumers to obey some rules (lowercase and no space).
  // - `literal` must only contain lower case ascii characters
  // there is one edge case where this could give false positives
  // test could contain a (non-ascii) char exactly 32 below literal
  // ##########################################################################
  inline bool equalsLiteral(const std::string& literal, const std::string test) {
    // Work directly on characters
    const char* src = test.c_str();
    const char* lit = literal.c_str();
    // There is a small chance that the search string
    // Is longer than the rest of the string to look at
    while (*lit && (*src == *lit || *src + 32 == *lit)) {
      ++src, ++lit;
    }
    // True if literal is at end
    // If not test was too long
    return *lit == 0;
  }
  // EO equalsLiteral

  // ##########################################################################
  // Returns whether [name] is the name of a pseudo-element
  // that can be written with pseudo-class syntax (CSS2 vs CSS3):
  // `:before`, `:after`, `:first-line`, or `:first-letter`
  // ##########################################################################
  inline bool isFakePseudoElement(const std::string& name)
  {
    return equalsLiteral("after", name)
      || equalsLiteral("before", name)
      || equalsLiteral("first-line", name)
      || equalsLiteral("first-letter", name);
  }

  // ##########################################################################
  // Names of pseudo selectors that take selectors as arguments,
  // and that are subselectors of their arguments.
  // For example, `.foo` is a superselector of `:matches(.foo)`.
  // ##########################################################################
  inline bool isSubselectorPseudo(const std::string& norm)
  {
    return equalsLiteral("any", norm)
      || equalsLiteral("matches", norm)
      || equalsLiteral("nth-child", norm)
      || equalsLiteral("nth-last-child", norm);
  }
  // EO isSubselectorPseudo

  // ###########################################################################
  // Pseudo-class selectors that take unadorned selectors as arguments.
  // ###########################################################################
  inline bool isSelectorPseudoClass(const std::string& test)
  {
    return equalsLiteral("not", test)
      || equalsLiteral("matches", test)
      || equalsLiteral("current", test)
      || equalsLiteral("any", test)
      || equalsLiteral("has", test)
      || equalsLiteral("host", test)
      || equalsLiteral("host-context", test);
  }
  // EO isSelectorPseudoClass

  // ###########################################################################
  // Pseudo-element selectors that take unadorned selectors as arguments.
  // ###########################################################################
  inline bool isSelectorPseudoElement(const std::string& test)
  {
    return equalsLiteral("slotted", test);
  }
  // EO isSelectorPseudoElement

  // ###########################################################################
  // Pseudo-element selectors that has binominals
  // ###########################################################################
  inline bool isSelectorPseudoBinominal(const std::string& test)
  {
    return equalsLiteral("nth-child", test)
      || equalsLiteral("nth-last-child", test);
  }
  // isSelectorPseudoBinominal

  // ###########################################################################
  // Returns [name] without a vendor prefix.
  // If [name] has no vendor prefix, it's returned as-is.
  // ###########################################################################
  inline std::string unvendor(const std::string& name)
  {
    if (name.size() < 2) return name;
    if (name[0] != '-') return name;
    if (name[1] == '-') return name;
    for (size_t i = 2; i < name.size(); i++) {
      if (name[i] == '-') return name.substr(i + 1);
    }
    return name;
  }
  // EO unvendor

  // ###########################################################################
  // Return [name] without pseudo and vendor prefix
  // ###########################################################################
  inline std::string pseudoName(std::string name)
  {
    size_t n = name[0] == ':' ? 1 : 0;
    std::replace(name.begin(), name.end(), '_', '-');
    if (name.size() < 2 + n) return name.substr(n);
    if (name[n] != '-') return name.substr(n);
    if (name[n + 1] == '-') return name.substr(n);
    for (size_t i = 2 + n; i < name.size(); i++) {
      if (name[i] == '-') return name.substr(i + 1);
    }
    return name.substr(n);
  }
  // EO pseudoName

  // ###########################################################################
  // ###########################################################################

}

#endif
