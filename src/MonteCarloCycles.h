// Brian Goldman

// This file uses a sampled version of the "Tarjan" algorithm to
// find random strongly-connected-components of the model's transition graph.
// These correspond to cycles the model can get in but cannot get out, such that
// all states in the cycle can reach all other in the cycle.
// This can work with either synchronously or asynchronously by modifying
// a line lower in this file marked by "TODO"
#ifndef MONTECARLOCYCLES_H_
#define MONTECARLOCYCLES_H_

#include <unordered_map>
#include <algorithm>
#include "Model.h"

class MonteCarloCycles {
 public:
  // "Stack Limit" is the maximum size of a component Tarjan can find before giving up.
  // This is designed to keep memory usage under control.
  MonteCarloCycles(const Model& model_, Random & random_, size_t stack_limit_)
      : model(model_),
        random(random_),
        stack_limit(stack_limit_) {
  }
  ;
  // Start from a random state, perform Tarjan until a stable cycle is found, then add it to the cycles
  void iterate();
  // Print out all of the cycles to the file
  void print(std::ostream& out);
 private:
  const Model& model;
  Random& random;
  size_t stack_limit;
  // A vector cycles, where each cycle is a vector of states
  // where each state is a vector of ints.
  vector<vector<vector<int>>> cycles;
  // Count how many times each cycle is encountered
  vector<int> cycle_seen;
  // Maps a state to the position in "cycle_seen" corresponding to that
  // state's previously found cycle
  std::unordered_map<vector<int>, size_t> state_in_cycle;
  // Performs the tarjan algorithm starting from start_state until the first
  // strongly connected component is found, or the stack limit is reached.
  // For more details see:
  // https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
  bool tarjan(const vector<int>& start_state);
};

// Helper data structure that stores all information needed by a stack level of tarjan.
struct tarjan_container {
  // The current model state for this level of the stack
  vector<int> state;
  // List of all outbound edges from "state" that haven't been explored yet.
  vector<vector<int>> unsearched_neighbors;
  // Index identifies what order this state was found in
  size_t index = 0;
  // low_link is the lowest index reachable by performing depth-first-search from "state"
  size_t low_link = 0;

  tarjan_container() = default;
  tarjan_container(const vector<int> & s, size_t i, const Model& model,
                   Random & random)
      : state(s),
        index(i),
        low_link(i) {
    //unsearched_neighbors.push_back(model.get_sync_next(state));
    // TODO To switch between synchronous and asynchronous, uncomment/comment the line above
    // and comment / uncomment the line below.
    unsearched_neighbors = model.get_clock_next_states(state);
    shuffle(unsearched_neighbors.begin(), unsearched_neighbors.end(), random);
  }
};

#endif /* MONTECARLOCYCLES_H_ */
