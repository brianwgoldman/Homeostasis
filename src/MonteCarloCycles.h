/*
 * MonteCarloCycles.h
 *
 *  Created on: Feb 9, 2016
 *      Author: goldman
 */

#ifndef MONTECARLOCYCLES_H_
#define MONTECARLOCYCLES_H_

#include <unordered_map>
#include "Model.h"

class MonteCarloCycles {
 public:
  MonteCarloCycles(const Model& model_, Random & random_, size_t stack_limit_) : model(model_), random(random_), stack_limit(stack_limit_) { };
  void iterate();
  void print(std::ostream& out);
 private:
  const Model& model;
  Random& random;
  size_t stack_limit;
  // A vector cycles, where each cycle is a vector of states
  // where each state is a vector of ints.
  vector<vector<vector<int>>> cycles;
  vector<int> cycle_seen;
  std::unordered_map<vector<int>, size_t> state_in_cycle;

  bool tarjan(const vector<int>& start_state);
};


#endif /* MONTECARLOCYCLES_H_ */
