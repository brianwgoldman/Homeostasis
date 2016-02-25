/*
 * WalkCycle.h
 *
 *  Created on: Feb 24, 2016
 *      Author: goldman
 */

#ifndef WALKCYCLE_H_
#define WALKCYCLE_H_
#include <vector>
using std::vector;
#include <unordered_set>
using std::unordered_set;

#include "Model.h"

class WalkCycle {
 public:
  WalkCycle(const Model& model_, Random & random_, size_t stack_limit_) : model(model_), random(random_), stack_limit(stack_limit_) { };
  void iterate();
  void print(std::ostream& out);
 private:
  const Model& model;
  Random& random;
  size_t stack_limit;
  // A vector cycles, where each cycle is a vector of states
  // where each state is a vector of ints.
  vector<vector<vector<int>>> cycles;
  vector<vector<int>> walk_until_cycle(const vector<int>& start);
  unordered_map<vector<int>, size_t> starts, successes;
  unordered_set<vector<int>> known_cyclic;
  vector<vector<int>> selectable;

  bool cort_cycle_check(const vector<vector<int>> & cycle) const;
};

#endif /* WALKCYCLE_H_ */
