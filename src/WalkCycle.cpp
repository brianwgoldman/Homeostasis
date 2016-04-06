/*
 * WalkCycle.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: goldman
 */

#include "WalkCycle.h"
#include <iostream>
using std::cout;
using std::endl;
#include <cassert>
#include <unordered_set>
#include <algorithm>

void WalkCycle::print(std::ostream& out) {
  for (size_t i=0; i < cycles.size(); i++) {
    for (const auto & state : cycles[i]) {
      model.print(state, out);
    }
    out << endl;
  }
  cout << "Total found: " << cycles.size() << endl;
  out << "# Total found: " << cycles.size() << endl;

  unordered_map<vector<int>, size_t> frequency;
  for (const auto & cycle : cycles) {
    for (const auto & state : cycle) {
      frequency[state]++;
    }
  }
  /*
  vector<std::pair<size_t, vector<int>>> sortable;
  for (const auto & pair : frequency) {
    if (pair.second > 3) {
      sortable.emplace_back(pair.second, pair.first);
    }
  }
  */
  vector<std::pair<double, vector<int>>> sortable;
  //cout << "Successes: " << successes.size() << " edge freq " << edge_frequency.size() << endl;
  for (const auto & pair : seen_count) {
    sortable.emplace_back(pair.second, pair.first);
    if (sortable.back().first < 100) {
      sortable.pop_back();
    }
  }
  sort(sortable.begin(), sortable.end());
  for (const auto pair : sortable) {
    cout << pair.first << ", ";
    model.print(pair.second, cout);
    for (const auto edge : edge_frequency.at(pair.second)) {
      cout << edge.second << ", ";
      model.print(edge.first, cout);
    }
    cout << endl;
  }
  cout << "Found cycle states: " << seen_count.size() << endl;
}

bool WalkCycle::cort_cycle_check(const vector<vector<int>> & cycle) const {
  size_t cort_position = model.find_position("CORT");
  assert(cort_position < model.size());

  std::unordered_set<int> cort_values;
  for (const auto & step : cycle) {
    cort_values.insert(step[cort_position]);
  }
  return cort_values.size() > 1;
}

void WalkCycle::record_edges(vector<vector<int>> & cycle) {
  for (size_t i=0; i < cycle.size(); i++) {
    auto & from = cycle[i];
    auto & to = cycle[(i+1) % cycle.size()];
    auto freq = edge_frequency.find(from);
    if (freq == edge_frequency.end()) {
      auto& in_table = edge_frequency[from];
      // Never seen this state before, so note all o fits possible edges
      for (const auto& next : model.get_async_next_states(from)) {
        in_table[next] = 0;
      }
      freq = edge_frequency.find(from);
    }
    freq->second[to]++;
  }
}

void WalkCycle::iterate() {
  vector<int> start;
  if (not needs_grind.empty()) {
    // Start from a node that we know needs exploring
    start = needs_grind.back();
    grind_count.back()++;
    if (grind_count.back() >= 1000) {
      grind_count.pop_back();
      needs_grind.pop_back();
    }
  } else {
    // Try to find a brand new cycle
    start = model.random_states(random);
  }
  auto cycle = walk_until_cycle(start);
  if (not cycle.empty()) {
    if (cort_cycle_check(cycle)) {
      record_edges(cycle);
      //cout << "Found cort cycle" << endl;
      cycles.emplace_back(cycle);
      for (const auto & step : cycle) {
        seen_count[step]++;
        // If this is the first time you've seen that node
        if (seen_count[step] == 1) {
          needs_grind.push_back(step);
          grind_count.push_back(0);
        }
      }
    }
  }
}


vector<vector<int>> WalkCycle::walk_until_cycle(const vector<int>& start) {
  vector<vector<int>> path;
  path.emplace_back(start);
  unordered_map<vector<int>, size_t> path_position;
  do {
    // Assign the previous back to a position
    path_position[path.back()] = path.size() - 1;
    auto options = model.get_async_next_states(path.back());
    if (options.empty()) {
      // You have reached a steady state, time to bail
     // cout << "Steady state reached, bailing" << endl;
      return {};
    }
    // pick one at random
    size_t choice = std::uniform_int_distribution<size_t>(0, options.size() - 1)(random);
    path.push_back(options[choice]);
    // Stop when the new back already has a position, or if the path gets too long
  } while (path_position.count(path.back()) == 0 and path.size() < stack_limit);
  if (path.size() >= stack_limit) {
    cout << "Stack Limited" << endl;
    return {};
  }
  //cout << "Cycle found" << endl;
  // At this point you know path.back() is in path twice. Everything
  // between those points is the cycle
  size_t repeated = path_position[path.back()];
  // Only have one copy of the repeated variable
  vector<vector<int>> cycle(path.begin() + repeated + 1, path.end());
  return cycle;
}
