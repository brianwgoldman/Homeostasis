// Brian Goldman
// A collection of tools that make life easier but don't fit into
// any other files
#ifndef UTILITIES_H_
#define UTILITIES_H_

// Use C++11 style random number generation
#include <random>
using Random=std::mt19937;


// This is taken from Boost to allow for hashing of vector<int>
template <class T>
inline void hash_combine(std::size_t & seed, const T & v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
  template<typename T> struct hash<vector<T>> {
    inline size_t operator()(const vector<T> & v) const {
      size_t seed = 0;
      for (const auto & elem : v) {
        hash_combine(seed, elem);
      }
      return seed;
    }
  };
}


#endif /* UTILITIES_H_ */
