#ifndef SASS_DART_HELPERS_H
#define SASS_DART_HELPERS_H

#include <vector>
#include <utility>
#include <iterator>
#include <functional>
#include "ast_helpers.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Returns a new list containing the elements between [start] and [end].
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  sass::vector<T> sublist(const sass::vector<T>& vec,
    size_t start, size_t end = sass::string::npos)
  {
    if (end == sass::string::npos) { end = vec.size(); }
    return sass::vector<T>(vec.begin() + start, vec.begin() + end);
  }

  /////////////////////////////////////////////////////////////////////////
  // Removes the objects in the range [start] inclusive to [end] exclusive.
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  void removeRange(sass::vector<T>& vec,
    size_t start, size_t end = sass::string::npos)
  {
    if (end == sass::string::npos) { end = vec.size(); }
    vec.erase(vec.begin() + start, vec.begin() + end);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  template <class T, class V>
  size_t indexOf(const sass::vector<T>& vec, const V& item)
  {
    for (size_t i = 0; i < vec.size(); i += 1) {
      if (ObjEqualityFn<T>(vec[i], item)) return i;
    }
    return sass::string::npos;
  }

  /////////////////////////////////////////////////////////////////////////
  // Flatten `vector<vector<T>>` to `vector<T>`
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  T flatten(const sass::vector<T>& all)
  {
    T flattened;
    for (const auto& sub : all) {
      std::copy(std::begin(sub), std::end(sub),
        std::back_inserter(flattened));
    }
    return flattened;
  }

  /////////////////////////////////////////////////////////////////////////
  // Expands each element of this Iterable into zero or more elements.
  // Calls a function on every element and ads all results to flat array
  /////////////////////////////////////////////////////////////////////////
  // Equivalent to dart `cnt.any`
  // Passes additional closure variables to `fn`
  template <class T, class U, typename ...Args>
  T expand(const T& cnt, U fn, Args... args) {
    T flattened;
    for (const auto& sub : cnt) {
      auto rv = fn(sub, args...);
      flattened.insert(flattened.end(),
        rv.begin(), rv.end());
    }
    return flattened;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  T flattenInner(const sass::vector<T>& vec)
  {
    T outer;
    for (const auto& sub : vec) {
      outer.emplace_back(std::move(flatten(sub)));
    }
    return outer;
  }
  // EO flattenInner


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  template <typename T>
  sass::vector<T> flattenVertically(sass::vector<sass::vector<T>> lists)
  {
    sass::vector<T> result;
    // Loop until all arrays are exhausted
    size_t lvl = 0; bool consumed = false; do {
      // aborts when nothing more can be consumed
      consumed = false;
      // loop over all arrays at the 1st level
      for (size_t i = 0, iL = lists.size(); i < iL; ++i) {
        // check for items to consume at 2nd level
        if (lists[i].size() > lvl) {
          // consume item at 2nd level depth
          result.emplace_back(lists[i][lvl]);
          // maybe we have some more
          consumed = true;
        }
      }
      // check next depth level
      ++lvl;
    }
    // abort once nothing is consumed
    while (consumed);
    // return flat list
    return result;
  }
  // EO flatVertically

  /////////////////////////////////////////////////////////////////////////
  // Equivalent to dart `cnt.any`
  // Passes additional closure variables to `fn`
  /////////////////////////////////////////////////////////////////////////
  template <class T, class U, typename ...Args>
  bool hasAny(const T& cnt, U fn, Args... args) {
    for (const auto& sub : cnt) {
      if (fn(sub, args...)) {
        return true;
      }
    }
    return false;
  }
  // EO hasAny

  /////////////////////////////////////////////////////////////////////////
  // Equivalent to dart `cnt.take(len).any`
  // Passes additional closure variables to `fn`
  /////////////////////////////////////////////////////////////////////////
  template <class T, class U, typename ...Args>
  bool hasSubAny(const T& cnt, size_t len, U fn, Args... args) {
    for (size_t i = 0; i < len; i++) {
      if (fn(cnt[i], args...)) {
        return true;
      }
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  // Default predicate for lcs algorithm
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  inline bool lcsIdentityCmp(const T& X, const T& Y, T& result)
  {
    // Assert equality
    if (!ObjEqualityFn(X, Y)) {
      return false;
    }
    // Store in reference
    result = X;
    // Return success
    return true;
  }
  // EO lcsIdentityCmp

  /////////////////////////////////////////////////////////////////////////
  // Longest common subsequence with predicate
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  sass::vector<T> lcs(
    const sass::vector<T>& X, const sass::vector<T>& Y,
    bool(*select)(const T&, const T&, T&) = lcsIdentityCmp<T>)
  {

    std::size_t m = X.size(), mm = X.size() + 1;
    std::size_t n = Y.size(), nn = Y.size() + 1;

    if (m == 0) return {};
    if (n == 0) return {};

    // MSVC does not support variable-length arrays
    // To circumvent, allocate one array on the heap
    // Then use a macro to access via double index
    // e.g. `size_t L[m][n]` is supported by gcc
    size_t* len = new size_t[mm * nn + 8];
    bool* acc = new bool[mm * nn + 8]{};
    T* res = new T[mm * nn + 8];

    #define LEN(x, y) len[(x) * nn + (y)]
    #define ACC(x, y) acc[(x) * nn + (y)]
    #define RES(x, y) res[(x) * nn + (y)]

    /* Following steps build L[m+1][n+1] in bottom up fashion. Note
      that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
    for (size_t i = 0; i <= m; i++) {
      for (size_t j = 0; j <= n; j++) {
        if (i == 0 || j == 0)
          LEN(i, j) = 0;
        else {
          ACC(i - 1, j - 1) = select(X[i - 1], Y[j - 1], RES(i - 1, j - 1));
          if (ACC(i - 1, j - 1))
            LEN(i, j) = LEN(i - 1, j - 1) + 1;
          else
            LEN(i, j) = std::max(LEN(i - 1, j), LEN(i, j - 1));
        }
      }
    }

    // Following code is used to print LCS
    sass::vector<T> lcs;
    std::size_t index = LEN(m, n);
    lcs.reserve(index);

    // Start from the right-most-bottom-most corner
    // and one by one store objects in lcs[]
    std::size_t i = m, j = n;
    while (i > 0 && j > 0) {

      // If current objects in X[] and Y are same,
      // then current object is part of LCS
      if (ACC(i - 1, j - 1))
      {
        // Put the stored object in result
        // Note: we push instead of unshift
        // Note: reverse the vector later
        // ToDo: is deque more performant?
        lcs.emplace_back(RES(i - 1, j - 1));
        // reduce values of i, j and index
        i -= 1; j -= 1; index -= 1;
      }

      // If not same, then find the larger of two and
      // go in the direction of larger value
      else if (LEN(i - 1, j) > LEN(i, j - 1)) {
        i--;
      }
      else {
        j--;
      }

    }

    // reverse now as we used emplace_back
    std::reverse(lcs.begin(), lcs.end());

    // Delete temp memory on heap
    delete[] len;
    delete[] acc;
    delete[] res;

    #undef LEN
    #undef ACC
    #undef RES

    return lcs;
  }
  // EO lcs

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
