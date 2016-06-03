/*
 * Cycles.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: goldman
 */

#include "Cycles.h"
#include <unordered_set>

Cycles::Cycles(const Model& model_) : model(model_) {

}

bool Cycles::increment(vector<int> & current) const {
  size_t index = 0;
  const auto & interactions = model.get_interactions();
  while (current[index] == interactions[index].upper_bound) {
    current[index] = interactions[index].lower_bound;
    index++;
    if (index >= current.size()) {
      return false;
    }
  }
  current[index]++;

  return true;
}

bool less_than(const vector<int> & left, const vector<int>& right) {
  for (int i=left.size() - 1; i >= 0; i--) {
    if (left[i] < right[i]) {
      return true;
    } else if (left[i] > right[i]) {
      return false;
    }
  }
  return false;
}

void Cycles::find_cycles(std::ostream& out) {
  model.print_header(out);

  std::unordered_set<vector<int>> known_cycle;

  vector<int> counter;
  for (const auto interaction : model.get_interactions()) {
    counter.push_back(interaction.lower_bound);
  }
  do {
    // If you have seen this step before, skip it
    if (known_cycle.count(counter)) {
      continue;
    }
    // copy the start states to the beginning of the path
    vector<vector<int>> path(1, counter);
    unordered_map<vector<int>, size_t> path_position;
    size_t end_of_path = 0;
    path_position[path.back()] = end_of_path;

    do {
      // advance the path by 1
      path.emplace_back(model.get_sync_next(path.back()));
      end_of_path++;
      auto result = path_position.insert({path.back(), end_of_path});
      // If this state already has a position in our path
      if (not result.second) {
        size_t start_of_cycle = result.first->second;
        out << end_of_path - start_of_cycle << std::endl;
        for (size_t i = start_of_cycle; i < end_of_path; i++) {
          model.print(path[i], out);
          known_cycle.insert(path[i]);
        }
        out << std::endl;
      }
      if (less_than(path.back(), path[0])) {
        break;
      }

    // loop as long as you haven't connected back to something we've seen before
    } while (known_cycle.count(path.back()) == 0);
  } while (increment(counter));
}
