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

  vector<STATE> reference;
  void make_move(size_t index, STATE newstate);
  size_t increment(size_t index);
  // Tracks how many interactions with "index" as its minimum dependency
  // currently require a change
  vector<int> index_needs_change;

  vector<int> target_needs_change;
  int total_changes_needed=0;

  void rebuild_changes_needed();
  /*
  // TODO Old
  // List of all moves, which are just collections of indices
  vector<vector<size_t>> moves;
  // Indices of single bit moves in the "moves" vector
  vector<size_t> single_bit_moves;
  // Lookup tables to find which moves effect what subfunctions, and vice versa
  vector<unordered_set<size_t>> move_to_sub, sub_to_move;
  vector<vector<size_t>> bit_to_sub;
  // For each move, figure out which bin it goes into
  vector<size_t> move_to_bin;
  // Keeps track of how many improving moves are in each bin
  vector<size_t> moves_in_bin;
  // Table storing the fitness effect of making a particular move
  vector<int> delta;
  // Count of current number of fitness improving moves
  int improving_moves;

  // Conversion lookups between the original index ordering and the
  // remapped ordering
  vector<int> new_to_org, org_to_new;

  // Time stamp of when the class was first given the landscape
  std::chrono::steady_clock::time_point start;

  // Construct all of the initial fitness effects of making moves
  void initialize_deltas();
  // Flip all of the bits associated with a given move
  void flip_move(size_t move_index);
  // Flips a given bit and updates all deltas and move_bin counts
  int make_flip(size_t index);

  // Performs the reordering of how enumeration is performed
  // to improve hyperplane skipping
  void remap();
  // Figure out what bin each move should be placed in, and set
  // up initial bin counts
  void bin_moves();
  */
};

#endif /* ENUMERATION_H_ */
