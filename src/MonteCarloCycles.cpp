/*
 * MonteCarloCycles.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: goldman
 */

#include "MonteCarloCycles.h"
#include <algorithm>
using std::endl;

void MonteCarloCycles::print(std::ostream& out) {
  for (size_t i=0; i < cycles.size(); i++) {
    out << cycle_seen[i] << endl;
    for (const auto & state : cycles[i]) {
      model.print(state, out);
    }
  }
}

void MonteCarloCycles::iterate() {
  unordered_map<vector<int>, size_t> state_to_index;
  tarjan(model.random_states(random));
}

struct tarjan_container {
  vector<vector<int>> unsearched_neighbors;
  size_t index=0;
  size_t low_link=0;
  vector<int> state;
  tarjan_container() = default;
  tarjan_container(const vector<int> & s, size_t i, const Model& model, Random & random) : index(i), low_link(i), state(s) {
    unsearched_neighbors = model.get_clock_next_states(state);
    shuffle(unsearched_neighbors.begin(), unsearched_neighbors.end(), random);
  }
};

#include <unordered_set>

bool MonteCarloCycles::tarjan(const vector<int>& start_state) {
  unordered_map<vector<int>, size_t> state_to_index;
  vector<tarjan_container> state_stack;
  size_t index = 0;
  state_stack.emplace_back(start_state, index, model, random);
  state_to_index[start_state] = index;
  vector<size_t> recursion_stack = {index};
  index++;
  while (not recursion_stack.empty()) {
    auto & state = state_stack[recursion_stack.back()];
    // if you have explored all of this thing's links
    if (state.unsearched_neighbors.empty()) {
      // If this state can't connect back up the stack, you found a sink
      if (state.low_link == state.index) {
        // Record the found cycle
        vector<vector<int>> cycle;
        for (size_t i=recursion_stack.back(); i < state_stack.size(); i++) {
          cycle.push_back(state_stack[i].state);
        }
        cycles.push_back(cycle);
        if (cycles.back().size() > 1) {
          std::cout << "It happened! " << cycles.back().size() << endl;
        }
        cycle_seen.push_back(1);
        for (const auto& s : cycles.back()) {
          state_in_cycle[s] = cycles.size() - 1;
        }
        return true;
      }
      // Pop the stack and keep going
      recursion_stack.pop_back();
      continue;
    }
    // More neighbors to explore
    auto & next = state.unsearched_neighbors.back();
    // If this state doesn't have an index yet
    auto known = state_to_index.find(next);
    if (known == state_to_index.end()) {
      // If this state is part of a cycle we've already detected
      auto seen = state_in_cycle.find(next);
      if (seen != state_in_cycle.end()) {
        cycle_seen[seen->second]++;
        // Stop, nothing new was found
        return false;
      }
      // Search has reached the size limit
      if (index >= stack_limit) {
        std::cout << "Stack limited, stopping early" << endl;
        return false;
      }
      // Add it to the recursion stack
      state_stack.emplace_back(next, index, model, random);
      state_to_index[next] = index;
      recursion_stack.push_back(index);
      index++;
      if (index % 10000 == 0) {
        std::cout << "Stack size: " << index << endl;
      }
    } else {
      state.low_link = std::min(state.low_link, state_stack[known->second].low_link);
      // Warning, after this line "next" is invalid
      state.unsearched_neighbors.pop_back();
    }
  }
  throw std::invalid_argument("Reached impossible state in Monte Carlo Cycles");
  return false;
}
