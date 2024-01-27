#ifndef SASS_ENVIRONMENT_KEY_HPP
#define SASS_ENVIRONMENT_KEY_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

namespace Sass {

  class EnvKey {

  private:

    // The original name
    sass::string _orig;
    // The normalized name
    sass::string _norm;

    // Lazy calculated hash
    mutable size_t _hash;

    // Called by constructors
    inline void normalize()
    {
      // Normalize string
      std::replace(
        _norm.begin(),
        _norm.end(),
        '_', '-');
    }

  public:

    // Empty constructor
    EnvKey() :
      _hash(0)
    {}

    // String copy constructor
    EnvKey(const sass::string& orig) :
      _orig(orig),
      _norm(orig),
      _hash(0)
    {
      normalize();
    }

    // String move constructor
    EnvKey(sass::string&& orig) :
      _orig(std::move(orig)),
      _norm(_orig),
      _hash(0)
    {
      normalize();
    }

    // Copy constructor
    EnvKey(const EnvKey& key) :
      _orig(key._orig),
      _norm(key._norm),
      _hash(key._hash)
    {
    }

    // Move constructor
    EnvKey(EnvKey&& key) noexcept :
      _orig(std::move(key._orig)),
      _norm(std::move(key._norm)),
      _hash(std::move(key._hash))
    {
    }

    // Copy assignment operator
    EnvKey& operator=(const EnvKey& key)
    {
      _orig = key._orig;
      _norm = key._norm;
      _hash = key._hash;
      return *this;
    }

    // Move assignment operator
    EnvKey& operator=(EnvKey&& key) noexcept
    {
      _orig = std::move(key._orig);
      _norm = std::move(key._norm);
      _hash = std::move(key._hash);
      return *this;
    }

    bool isPrivate() const {
      return _norm[0] == '-';
    }

    // Compare normalization forms
    bool operator==(const EnvKey& rhs) const
    {
      return norm() == rhs.norm();
    }

    // Compare normalization forms
    bool operator<(const EnvKey& rhs) const
    {
      return norm() < rhs.norm();
    }

    // Simple helper
    bool empty() const
    {
      return _norm.empty();
    }

    // Simple constant getter functions
    const sass::string& orig() const { return _orig; }
    const sass::string& norm() const { return _norm; }

    // Calculate only on demand
    size_t hash() const {
      if (_hash == 0) {
        _hash = MurmurHash2(
          (void*)_norm.c_str(),
          (int)_norm.size(),
          getHashSeed());
      }
      return _hash;
    }

  };

  struct hashEnvKey {
    inline size_t operator()(const EnvKey& str) const
    {
      return str.hash();
    }
  };

  struct equalsEnvKey {
    bool operator() (const EnvKey& lhs, const EnvKey& rhs) const {
      return lhs.norm() == rhs.norm();
    }
  };

  struct hashString {
    inline size_t operator()(const sass::string& str) const
    {
      return MurmurHash2(
        (void*)str.c_str(),
        (int)str.size(),
        getHashSeed());
    }
  };

  struct equalsString {
    bool operator() (const sass::string& lhs, const sass::string& rhs) const {
      return lhs == rhs;
    }
  };

};

#endif
