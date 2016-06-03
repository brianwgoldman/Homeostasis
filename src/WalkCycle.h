// Brian Goldman
// Perform a random walk of the model's transition graph
// until you loop back on a state you've already seen, then
// record that as a cycle. Outputs all found cycles. It will
// also attempt to start from every state it thinks is part
// of a cycle 1000 times and count how often that state.
#ifndef WALKCYCLE_H_
#define WALKCYCLE_H_
#include <vector>
using std::vector;
#include <unordered_set>
using std::unordered_set;

#include "Model.h"

class WalkCycle {
 public:
  // stack_limit is designed to prevent excessive memory usage by stopping walks
  // if they go on too long.
  WalkCycle(const Model& model_, Random & random_, size_t stack_limit_) : model(model_), random(random_), stack_limit(stack_limit_) { };
  // Performs a random walk until that walk loops back on itself and records
  // information about the walk. Starts either from a random state or from
  // a previous state that was part of a cycle
  void iterate();
  void print(std::ostream& out);
 private:
  const Model& model;
  Random& random;
  size_t stack_limit;
  // A vector cycles, where each cycle is a vector of states
  // where each state is a vector of ints.
  vector<vector<vector<int>>> cycles;
  // From a start state, perform a random DFS until you loop back to a state
  // you've already seen during this walk. Returns an empty vector if you
  // reach a steady state
  vector<vector<int>> walk_until_cycle(const vector<int>& start);

  // Used to track if a state needs further exploration of its edges
  vector<vector<int>> needs_grind;
  vector<size_t> grind_count;
  unordered_map<vector<int>, size_t> seen_count;
  // Given a cycle, update edge_frequency
  void record_edges(vector<vector<int>> & cycle);
  // edge_frequency[X][Y] is how often a transition from X to Y was found
  // to be part of a cort cycle
  unordered_map<vector<int>, unordered_map<vector<int>, size_t>> edge_frequency;
  // Determines if this cycle contains at least two unique levels of CORT
  bool cort_cycle_check(const vector<vector<int>> & cycle) const;
};

#endif /* WALKCYCLE_H_ */
