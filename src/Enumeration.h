// Brian Goldman

// This will find all stable states of the model, where a "stable state"
// is any configuration in which no interaction wants to change.
// Uses Hyperplane Elimination to perform this very quickly.

#ifndef ENUMERATION_H_
#define ENUMERATION_H_

#include "Model.h"
#include <ostream>
#include <chrono>
#include <unordered_set>

class Enumeration {
 public:
  // Set up initial information based on the model
  Enumeration(const Model & model_);
  // Perform the enumeration, writing all of the steady states
  // to the "out" stream.
  void enumerate(std::ostream& out);
 protected:
  const Model& model;
  size_t length;
  // affects_of[X] gives you the set of affected positions when
  // "X" is changed
  vector<std::unordered_set<size_t>> affects_of;

  // The current settings for all variables
  vector<int> reference;
  // Modifies reference[index] to be "newstate" and updates auxiliary data structures.
  void make_move(size_t index, int newstate);
  // Advance index as far as you can go without skipping a potential steady state
  size_t increment(size_t index);
  // Tracks how many interactions with "index" as its minimum dependency
  // currently require a change
  vector<int> index_needs_change;
  // Tracks if a target needs to be changed (always boolean, but int is faster).
  vector<int> target_needs_change;
  // If this value is 0, you are in a steady state
  int total_changes_needed;

  void rebuild_changes_needed();
};

#endif /* ENUMERATION_H_ */
