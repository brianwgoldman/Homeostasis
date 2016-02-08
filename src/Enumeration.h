// Brian Goldman

// This class is designed to find all local optima
// of a given MK Landscape. Allows for configuring
// the r-bit hamming radius of the local optima
// (how many bit flips are required to reach an improving
// solution), as well has turning off hyperplane elimination
// and reordering.

#ifndef ENUMERATION_H_
#define ENUMERATION_H_

#include "Model.h"
#include <ostream>
#include <chrono>
#include <unordered_set>

class Enumeration {
 public:
  // Set up initial information based on the landscape and the
  // desired hamming ball radius
  Enumeration(const Model & model_);
  // Perform the landscape enumeration, writing all of the local optima to the
  // "out" stream.
  void enumerate(std::ostream& out);
 protected:
  const Model& model;
  size_t length;
  vector<std::unordered_set<size_t>> affects_of;

  vector<int> reference;
  void make_move(size_t index, int newstate);
  size_t increment(size_t index);
  // Tracks how many interactions with "index" as its minimum dependency
  // currently require a change
  vector<int> index_needs_change;

  vector<int> target_needs_change;
  int total_changes_needed=0;

  void rebuild_changes_needed();
};

#endif /* ENUMERATION_H_ */
