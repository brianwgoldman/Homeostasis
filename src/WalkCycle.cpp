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
  for (const auto & pair : successes) {
    if (pair.second > 0) {
      sortable.emplace_back(static_cast<double>(pair.second) / starts.at(pair.first), pair.first);
      if (sortable.back().first < 0.01) {
        sortable.pop_back();
      }
    }
  }
  sort(sortable.begin(), sortable.end());
  for (const auto pair : sortable) {
    cout << pair.first << " / " << starts[pair.second] <<  ": ";
    model.print(pair.second, cout);
  }
  cout << "Found cycle states: " << selectable.size() << endl;
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

void WalkCycle::iterate() {
  // Try to find a brand new cycle
  auto start = model.random_states(random);
  auto cycle = walk_until_cycle(start);
  if (not cycle.empty()) {

    if (cort_cycle_check(cycle)) {
      cout << "Found cort cycle" << endl;
      cycles.emplace_back(cycle);
      for (const auto & step : cycle) {
        auto result = known_cyclic.insert(step);
        // if it was inserted
        if (result.second) {
          selectable.push_back(step);
        }
      }
    }
  }
  return;
  if (selectable.empty()) {
    return;
  }
  //size_t choice = std::uniform_int_distribution<size_t>(0, selectable.size() - 1)(random);
  for(size_t i = selectable.size() - 1; i > 0; i--) {
    if (starts[selectable[i]] <= starts[selectable[i-1]]) {
      break;
    }
    std::swap(selectable[i], selectable[i-1]);
  }
  size_t choice = selectable.size() - 1;
  /*
  for (size_t i=1; i < selectable.size(); i++) {
    if (starts[selectable[choice]] > starts[selectable[i]]) {
      choice = i;
    }
  }
  */
  auto smart_cycle = walk_until_cycle(selectable[choice]);
  starts[selectable[choice]]++;
  if (not smart_cycle.empty()) {
    // If you looped back to yourself or you found a cort cycle
    if (cort_cycle_check(smart_cycle)) {
      //if (smart_cycle.back() == selectable[choice]) {
        cout << "---------Success!-----------" << endl;
        successes[selectable[choice]]++;
      //}
      for (const auto & step : smart_cycle) {
        auto result = known_cyclic.insert(step);
        // if it was inserted
        if (result.second) {
          selectable.push_back(step);
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
