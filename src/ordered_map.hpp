/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ORDERED_MAP_H
#define SASS_ORDERED_MAP_H

#include <vector>
#include <unordered_map>
#include "memory.hpp"

// #include "../../hopscotch-map/include/tsl/hopscotch_map.h"
// #include "../../hopscotch-map/include/tsl/hopscotch_set.h"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Very simple and limited container for insertion ordered hash map.
  // Piggy-back implementation on std::unordered_map and std::vector.
  /////////////////////////////////////////////////////////////////////////
  // In order to support assignable value references, we can only store the 
  // value in one container. We can't reference values from one container in
  // another, since the pointer would be invalidated, once a container needs
  // re-allocation. To fix this we need a soft reference. Therefore we only
  // store the index into the list vector on the hash-map. This makes all 
  // access operations constant, but makes erasing of items, or insertion 
  // not at the end (which is not yet supported) in the worst case O(n), as
  // we need to adjust the indexes on the hash-map after the modified item.
  /////////////////////////////////////////////////////////////////////////
  template<
    class TKey,
    class TVal,
    class Hash = std::hash<TKey>,
    class KeyEqual = std::equal_to<TKey>,
    class ListAllocator = Sass::Allocator<std::pair<TKey, TVal>>,
    class MapAllocator = Sass::Allocator<std::pair<const TKey, size_t>>
  >
  class ordered_map {

    using map_type = typename std::unordered_map <
      TKey, size_t, Hash, KeyEqual, MapAllocator >;
    using pair_type = typename std::pair<TKey, TVal>;
    using list_type = typename std::vector<pair_type, ListAllocator>;

    private:

      // The main unordered map for key access
      map_type _map;

      // The insertion ordered list of kv-pairs
      list_type _list;

      // Instance of equality functor
      const KeyEqual _keyEqual;

    public:

      // Default constructor
      ordered_map() :
        _map(map_type()),
        _list(list_type()),
        _keyEqual(KeyEqual())
      {}

      // Explicit destructor for rule of three
      // ~ordered_map() {}

      // The copy constructor for rule of three
      ordered_map(const ordered_map* ptr) :
        _map(ptr->_map),
        _list(ptr->_list),
        _keyEqual(ptr->_keyEqual)
      {}

      // The copy constructor for rule of three
      ordered_map(const ordered_map& other) :
        _map(other._map),
        _list(other._list),
        _keyEqual(other._keyEqual)
      {}

      // The move constructor for rule of three
      ordered_map(ordered_map&& other) :
        _map(std::move(other._map)),
        _list(std::move(other._list)),
        _keyEqual(other._keyEqual)
      {}

      // The copy assignment operator for rule of three
      ordered_map& operator= (const ordered_map& other)
      {
        _map = other._map;
        _list = other._list;
        // _keyEqual = other._keyEqual;
        return *this;
      }

      // The move assignment operator for rule of three
      ordered_map& operator= (ordered_map&& other)
      {
        _map = std::move(other._map);
        _list = std::move(other._list);
        // _keyEqual = other._keyEqual;
        return *this;
      }

      /////////////////////////////////////////////////////////////////////////
      // Implement vector API partially (return pairs)
      /////////////////////////////////////////////////////////////////////////

      // Returns number of pairs (or keys).
      // Normal maps report double the size.
      size_t size() const
      {
        return _list.size();
      }
      // EO size

      // Returns true if map is empty
      bool empty() const
      {
        return _list.empty();
      }
      // EO empty

      // Get kv_pair for last item.
      // Throws out of boundary exception.
      pair_type& front()
      {
        return _list.front();
      }
      // EO front

      // Get kv_pair for last item.
      // Throws out of boundary exception.
      pair_type& back()
      {
        return _list.back();
      }
      // EO back

      /////////////////////////////////////////////////////////////////////////
      // Implement unordered_map API partially
      /////////////////////////////////////////////////////////////////////////

      // Returns 1 if the key exists
      size_t count(const TKey& key) const {
        return _map.count(key);
      }
      // EO count(key)

      // Find key and return ordered list iterator
      typename list_type::const_iterator find(const TKey& key) const {
        typename map_type::const_iterator existing = _map.find(key);
        if (existing == _map.end()) return _list.end();
        return _list.begin() + existing->second;
      }
      // EO find(key) const

      // Find key and return ordered list iterator
      typename list_type::iterator find(const TKey& key) {
        typename map_type::iterator existing = _map.find(key);
        if (existing == _map.end()) return _list.end();
        return _list.begin() + existing->second;
      }
      // EO find(key)

      /////////////////////////////////////////////////////////////////////////
      // Implement mixed manipulation API
      /////////////////////////////////////////////////////////////////////////

      // Append a new key/value pair
      void push_back(const pair_type& kv, bool overwrite = false)
      {
        typename map_type::iterator existing = _map.find(kv.first);
        // Check if key is already existing
        if (existing != _map.end()) {
          if (overwrite) {
            size_t idx = existing->second;
            _list[idx].second = kv.second;
          }
          else {
            throw std::logic_error("Key already exists");
          }
        }
        // Append new key and value
        else {
          _map[kv.first] = size();
          _list.emplace_back(kv);
        }
      }
      // EO push_back(kv_pair)

      // Append a new key/value pair
      void push_back(const TKey& key, const TVal& value, bool overwrite = false)
      {
        push_back(std::make_pair(key, value), overwrite);
      }
      // EO push_back(key, value)

      // Overwrite existing item or append it
      void set(const TKey& key, const TVal& value)
      {
        push_back(key, value, true);
      }
      // EO set(key, value)

      // Remove item by key
      bool erase(const TKey& key) {
        // Remove from map
        _map.erase(key);
        // Find the position in the array
        for (size_t i = 0; i < size(); i += 1) {
          if (_keyEqual(key, _list[i].first)) {
            _list.erase(_list.begin() + i);
            // Adjust all indexes after the removal
            for (size_t j = i; j < size(); j += 1) {
              --_map[_list[j].first]; // reduce by one
            }
            return true;
          }
        }
        return false;
      }
      // EO erase(key)

      // Remove item by index
      bool erase(size_t idx) {
        // Gracefully handle out of bound
        if (idx < 0 || idx >= size()) return false;
        // Remove key to index
        _map.erase(_list[idx].first);
        // Remove key / value pair
        _list.erase(_list.begin() + idx);
        // Adjust all indexes after the removal
        for (size_t j = idx; j < size(); j += 1) {
          --_map[_list[j].first]; // reduce by one
        }
        return true;
      }
      // EO erase(index)

      // Get item from map, if missing it will
      // be created and default initialized.
      TVal& operator[](const TKey& key) {
        typename map_type::iterator existing = _map.find(key);
        // Check if key is already existing
        if (existing != _map.end()) {
          return _list[existing->second].second;
        }
        _map[key] = _list.size();
        // We must default initialize the value!
        _list.emplace_back(std::make_pair(key, TVal()));
        return _list.back().second;
      }
      // EO operator[](key)

      // Get item from list by index.
      pair_type& operator[](size_t idx) {
        return _list[idx];
      }
      // EO operator[](index)

      // Get item from list by index.
      // Throws out of boundary error.
      pair_type& at(size_t idx) {
        return _list.at(idx);
      }
      // EO at(index)

      /////////////////////////////////////////////////////////////////////////
      // Some additional stuff
      /////////////////////////////////////////////////////////////////////////

      // Reserve memory
      void reserve(size_t size)
      {
        _map.reserve(size);
        _list.reserve(size);
      }
      // EO reserve

      /////////////////////////////////////////////////////////////////////////
      // Some syntax sugar API
      /////////////////////////////////////////////////////////////////////////

      // Note that this creates a new array every time.
      // Only call it if you really want to have a copy.
      sass::vector<TKey> keys() const {
        sass::vector<TKey> keys;
        keys.reserve(size());
        for (const pair_type& kv : _list) {
          keys.emplace_back(kv.first);
        }
        return keys;
      }
      // EO keys

      // Note that this creates a new array every time.
      // Only call it if you really want to have a copy.
      sass::vector<TVal> values() const {
        sass::vector<TVal> values;
        values.reserve(size());
        for (const pair_type& kv : _list) {
          values.emplace_back(kv.second);
        }
        return values;
      }
      // EO values

      /////////////////////////////////////////////////////////////////////////
      // Create iterator aliases for ourself
      /////////////////////////////////////////////////////////////////////////

      using iterator = typename list_type::iterator;
      using const_iterator = typename list_type::const_iterator;
      using reverse_iterator = typename list_type::reverse_iterator;
      using const_reverse_iterator = typename list_type::const_reverse_iterator;

      /////////////////////////////////////////////////////////////////////////
      // Implement iterator functions
      /////////////////////////////////////////////////////////////////////////

      iterator end() { return _list.end(); }
      iterator begin() { return _list.begin(); }
      reverse_iterator rend() { return _list.rend(); }
      reverse_iterator rbegin() { return _list.rbegin(); }
      const_iterator end() const { return _list.end(); }
      const_iterator begin() const { return _list.begin(); }
      const_reverse_iterator rend() const { return _list.rend(); }
      const_reverse_iterator rbegin() const { return _list.rbegin(); }

  };
  // EO ordered_map

}

#endif
