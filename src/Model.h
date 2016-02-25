/*
 * Model.h
 *
 *  Created on: Jan 21, 2016
 *      Author: goldman
 */

#ifndef MODEL_H_
#define MODEL_H_
#include <vector>
using std::vector;
using std::size_t;
#include <iostream>
using std::string;
#include <unordered_map>
using std::unordered_map;
#include <fstream>

#include <random>
using Random=std::mt19937;

struct Interaction {
  size_t target;
  vector<size_t> activators;
  vector<size_t> inhibitors;
  size_t minimum_dependency;
  int lower_bound;
  int upper_bound;


  string target_name;
  vector<string> activator_names;
  vector<string> inhibitor_names;
  int get_next_state(const vector<int>& current_states) const;
};

class Model {
 public:
  Model(const string filename);
  virtual ~Model() = default;
  const vector<Interaction>& get_interactions() const {
    return interactions;
  }
  vector<int> random_states(Random& random) const;
  vector<int> get_next(const vector<int>& current_states) const;
  vector<vector<int>> get_async_next_states(const vector<int>& current_states) const;
  const size_t size() const {
    return interactions.size();
  }
  void print(const vector<int>& current_state, std::ostream& out=std::cout) const;
  void print_header(std::ostream& out=std::cout) const;
  size_t find_position(const string& name) const;
 private:
  vector<Interaction> interactions;

  void load_old_format(const string filename);
  void load_csv(const string filename);
  void load_post_format(const string filename);

  void reorganize();

  // Tools for converting in and out of readable formats
  unordered_map<string, size_t> name_to_position;
  vector<string> position_to_name;
  vector<string> original_ordering;
};


// TODO Move this stuff somwhere better

// This is taken from Boost to allow for hashing of vector<int>
template <class T>
inline void hash_combine(std::size_t & seed, const T & v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
/*
template<typename S, typename T> struct hash<pair<S, T>> {
    inline size_t operator()(const pair<S, T> & v) const {
      size_t seed = 0;
      hash_combine(seed, v.first);
      hash_combine(seed, v.second);
      return seed;
    }
  };
*/
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


#endif /* MODEL_H_ */
