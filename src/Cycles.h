/*
 * Cycles.h
 *
 *  Created on: Jan 27, 2016
 *      Author: goldman
 */

#ifndef CYCLES_H_
#define CYCLES_H_

#include "Model.h"

class Cycles {
 public:
  Cycles(const Model& model_);
  void find_cycles(std::ostream& out);
 private:
  const Model& model;
  bool increment(vector<int> & current) const;

};

// TODO Move this stuff somwhere better

// This is taken from Boost to allow for hashing of vector<int>
template <class T>
inline void hash_combine(std::size_t & seed, const T & v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
  template<typename S, typename T> struct hash<pair<S, T>> {
    inline size_t operator()(const pair<S, T> & v) const {
      size_t seed = 0;
      hash_combine(seed, v.first);
      hash_combine(seed, v.second);
      return seed;
    }
  };

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


#endif /* CYCLES_H_ */
