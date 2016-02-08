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


struct Interaction {
  size_t target;
  vector<size_t> activators;
  vector<size_t> inhibitors;
  size_t minimum_dependency;


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
  const vector<std::pair<int, int>>& get_bounds() const {
    return bounds;
  }
  vector<int> get_next(const vector<int>& current_states) const;
  const size_t size() const {
    return interactions.size();
  }
  void print(const vector<int>& current_state, std::ostream& out=std::cout) const;
  void print_header(std::ostream& out=std::cout) const;
 private:
  vector<Interaction> interactions;
  vector<std::pair<int, int>> bounds;

  void load_old_format(const string filename);
  void load_csv(const string filename);

  void reorganize();

  // Tools for converting in and out of readable formats
  unordered_map<string, size_t> name_to_position;
  vector<string> position_to_name;
  vector<string> original_ordering;
};

#endif /* MODEL_H_ */
