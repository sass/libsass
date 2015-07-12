#include "ast.hpp"
#include "memory_manager.hpp"
#include "debugger.hpp"

namespace Sass {
  using namespace std;

  template <typename T>
  Memory_Manager<T>::Memory_Manager(size_t size)
  : nodes(vector<T*>())
  {
    // cerr << "pull up " << this << endl;
    nodes.reserve(size);
  }

  template <typename T>
  Memory_Manager<T>::~Memory_Manager()
  {
    // cerr << "tear down " << this << endl;
    for (size_t i = 0, S = nodes.size(); i < S; ++i) {
      // cerr << "-- " << this << " - " << nodes[i] << endl;
      delete nodes[i];
    }
  }

  template <typename T>
  T* Memory_Manager<T>::operator()(T* np)
  {
    nodes.push_back(np);
    // cerr << "++ " << this << " - " << np << endl;
    return np;
  }

  template <typename T>
  bool Memory_Manager<T>::has(T* np)
  {
    return find(nodes.begin(), nodes.end(), np) != nodes.end();
  }

  template <typename T>
  void Memory_Manager<T>::remove(T* np)
  {
    nodes.erase(find(nodes.begin(), nodes.end(), np));
  }

  template <typename T>
  void Memory_Manager<T>::destroy(T* np)
  {
    if (Map* m = dynamic_cast<Map*>(np)) {
      auto cur = m->begin();
      while(cur != m->end())
      {
        if (has(cur->first))
          destroy(cur->first);
        if (has(cur->second))
          destroy(cur->second);
        ++ cur;
      }
    }
    else if (List* l = dynamic_cast<List*>(np)) {
      for (size_t i = 0, L = l->length(); i < L; ++i) {
       // if (has((*l)[i])) destroy((*l)[i]);
      }
    }
    nodes.erase(find(nodes.begin(), nodes.end(), np));
    delete np;
  }

  // compile implementation for AST_Node
  template class Memory_Manager<AST_Node>;

}

