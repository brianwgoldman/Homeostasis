// Brian Goldman
// This provides the tools for interacting with our model of how they biochemistry
// works. Can read input files, write output files, and give a vector of "states"
// can tell you both synchronous and asynchronous updates to those states.
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

#include "Utilities.h"

// Structure for dictating how a variable (e.g. GnRH) interacts with other variables
struct Interaction {
  // Position and name this interaction controls
  size_t target;
  string target_name;
  // Positions and names of its activators
  vector<size_t> activators;
  vector<string> activator_names;
  // Positions and names of its inhibitors
  vector<size_t> inhibitors;
  vector<string> inhibitor_names;
  int lower_bound;
  int upper_bound;

  // Returns the value "current_states[target]" should be if this interaction is updated.
  int get_next_state(const vector<int>& current_states) const;
  // Reads from "current_states" and performs trinary logic to determine
  // if "target" should be considered activated (>0) inhibitied (<0) or neutral (==0).
  int get_direction_of_change(const vector<int>& current_states) const;
  // Used by fast enumeration, tracks the lowest index of all of this interaction's interactions.
  size_t minimum_dependency;
};

// Stores a collection of interactions and provides functions based on those interactions
class Model {
 public:
  // Reads in a file and sets up the ordering of interactions.
  Model(const string filename);
  virtual ~Model() = default;
  const vector<Interaction>& get_interactions() const {
    return interactions;
  }
  // Randomly create a vector of states such that each
  // variable is set within its allowed range.
  vector<int> random_states(Random& random) const;
  // Given a vector of states, return the result of doing a synchronous update
  vector<int> get_sync_next(const vector<int>& current_states) const;
  // Give a vector of states, return all possible states after performing an asychronous update.
  vector<vector<int>> get_async_next_states(const vector<int>& current_states) const;
  // Given a vector of states such that "CLOCK" is an interaction,
  // return all possible asychronous updates. "CLOCK" changes only if no variables currently
  // allowed to change want to change and at least one variable not allowed to change wants to.
  vector<vector<int>> get_clock_next_states(const vector<int>& current_states) const;
  const size_t size() const {
    return interactions.size();
  }
  // Print out a state in the original order it was read in.
  void print(const vector<int>& current_state, std::ostream& out=std::cout) const;
  // Print out column headers for this model
  void print_header(std::ostream& out=std::cout) const;
  // Read in a state as written by "print"
  vector<int> load_state(string line) const;
  // Return the index of a variable by name, -1 if that name isn't in the model.
  size_t find_position(const string& name) const;
 private:
  // All of the interactions in the problem
  vector<Interaction> interactions;

  // Loads a .csv file
  void load_csv(const string filename);
  // Loads files with the form: "GRD = CORT PROMOTES GR PROMOTES"
  void load_post_format(const string filename);
  // Puts interactions into an order conducive to fast enumeration
  void reorganize();

  // Tools for converting in and out of readable formats
  unordered_map<string, size_t> name_to_position;
  vector<string> position_to_name;
  vector<string> original_ordering;
};


#endif /* MODEL_H_ */
