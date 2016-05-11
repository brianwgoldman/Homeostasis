/*
 * Model.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: goldman
 */

#include "Model.h"
#include <algorithm>
using std::max_element;
#include <sstream>
#include <exception>
using std::invalid_argument;
using std::to_string;
#include <unordered_set>
using std::unordered_set;
#include <cassert>
#include <sstream>
using std::istringstream;

int Interaction::get_next_state(const vector<int>& current_states) const {
  vector<int> activator_states;
  for (const auto index : activators) {
    activator_states.push_back(current_states[index]);
  }
  vector<int> inhibitor_states;
  for (const auto index : inhibitors) {
    inhibitor_states.push_back(current_states[index]);
  }
  // Line 2 in Equation 2
  if (inhibitors.size() == 0) {
    if (activators.size() == 0) {
      return current_states[target];
    }
    return *std::max_element(activator_states.begin(), activator_states.end());

  // Line 3 in Equation 2
  } else if (activators.size() == 0) {
    auto result = *std::max_element(inhibitor_states.begin(), inhibitor_states.end());
    // Perform negation
    return -result;
  // Line 1 in Equation 2
  } else {
    auto active_aggregate = *std::max_element(activator_states.begin(), activator_states.end());
    auto inhibit_aggregate = *std::max_element(inhibitor_states.begin(), inhibitor_states.end());
    // Activated is > 0
    // TODO Update this if range changes
    if (active_aggregate > 0 and inhibit_aggregate <= 0) {
      return active_aggregate;
    } else if (inhibit_aggregate > 0 and active_aggregate <= 0) {
      return -inhibit_aggregate;
    } else {
      return 0;
    }
  }
}

vector<int> Model::get_next(const vector<int>& current_states) const {
  vector<int> result(current_states);
  for (const auto & interaction : interactions) {
    int next_state = interaction.get_next_state(current_states);
    int current = result[interaction.target];
    // This handles the "gradual change" idea
    if (current < next_state) {
      next_state = current + 1;
    } else if (current > next_state) {
      next_state = current - 1;
    }
    result[interaction.target] = next_state;
  }
  return result;
}

vector<vector<int>> Model::get_async_next_states(const vector<int>& current_states) const {
  vector<vector<int>> result;
  for (const auto & interaction : interactions) {
    int next_state = interaction.get_next_state(current_states);
    int current = current_states[interaction.target];
    // This handles the "gradual change" idea
    if (current < next_state) {
      next_state = current + 1;
    } else if (current > next_state) {
      next_state = current - 1;
    }
    if (next_state != current) {
      result.push_back(current_states);
      result.back()[interaction.target] = next_state;
    }
  }
  return result;
}

vector<vector<int>> Model::get_clock_next_states(const vector<int>& current_states) const {
  // This is the brain
  unordered_set<string> brain = {"LH/FSH", "ACTH", "GnRH", "CRH"};

  // Find the CLOCK
  size_t clock=interactions.size();
  bool brain_phase = false;
  for (const auto& interaction: interactions) {
    if (interaction.target_name == "CLOCK") {
      clock = interaction.target;
      brain_phase = current_states[clock] > 0;
    }
  }
  assert(clock < interactions.size());
  vector<vector<int>> result;
  bool off_phase_update = false;
  for (const auto & interaction : interactions) {
    if (interaction.target_name == "CLOCK") {
      // If this is the clock, skip it
      continue;
    }
    bool is_brain = brain.count(interaction.target_name) == 1;
    int next_state = interaction.get_next_state(current_states);
    int current = current_states[interaction.target];
    // This handles the "gradual change" idea
    if (current < next_state) {
      next_state = current + 1;
    } else if (current > next_state) {
      next_state = current - 1;
    }
    if (next_state != current) {
      if (brain_phase == is_brain) {
        // If this update is "on phase", create the resulting state
        result.push_back(current_states);
        result.back()[interaction.target] = next_state;
      } else {
        // This update is "off phase", e.g. its currently blood's turn
        // but the brain wants to update.
        off_phase_update = true;
      }
    }
  }
  if (off_phase_update and result.size() == 0) {
    // Advance the clock because we know eventually there will be an update
    result.push_back(current_states);
    result.back()[clock] = not brain_phase;
  }
  return result;
}

vector<int> Model::random_states(Random& random) const {
  vector<int> result(interactions.size(), 0);
  for (size_t i=0; i < interactions.size(); i++) {
    const auto & interaction = interactions[i];
    // generate a random number within the bounds
    std::uniform_int_distribution<int> dist(interaction.lower_bound, interaction.upper_bound);
    result[i] = dist(random);
  }
  return result;
}

// TODO Remove
using std::cout;
using std::endl;
Model::Model(string filename) {
  if (filename.substr(filename.size()-3, 3) == "csv") {
    cout << "Loading CSV" << endl;
    load_csv(filename);
  } else {
    load_post_format(filename);
  }

  position_to_name.resize(interactions.size());
  original_ordering.resize(interactions.size());

  // Assign names to numeric positions
  for (size_t i=0; i < interactions.size(); i++) {
    auto name = interactions[i].target_name;
    original_ordering[i] = name;
    //position_to_name[i] = name;
    //auto result = name_to_position.insert({name, i});
    //if (not result.second) {
    //  throw invalid_argument("Input file had two lines for the same target: " + name);
    //}
  }
  // TODO This might need to move after converting to numeric positions
  reorganize();

  // Convert names to numeric positions
  for (auto & interaction : interactions) {
    interaction.target = name_to_position[interaction.target_name];
    size_t min_dep = interaction.target;
    for (const auto name : interaction.activator_names) {
      auto result = name_to_position.find(name);
      if (result == name_to_position.end()) {
        throw invalid_argument("Input file had " +interaction.target_name + " have activator " + name + " that has no line of its own.");
      }
      interaction.activators.push_back(result->second);
      min_dep = std::min(min_dep, interaction.activators.back());
    }
    for (const auto name : interaction.inhibitor_names) {
      auto result = name_to_position.find(name);
      if (result == name_to_position.end()) {
        throw invalid_argument("Input file had " +interaction.target_name + " have inhibitor " + name + " that has no line of its own.");
      }
      interaction.inhibitors.push_back(result->second);
      min_dep = std::min(min_dep, interaction.inhibitors.back());
    }
    interaction.minimum_dependency = min_dep;
  }
  size_t index = 0;
  while (index < interactions.size()) {
    std::swap(interactions[index], interactions[interactions[index].target]);
    if (index == interactions[index].target) {
      index++;
    }
  }

  std::cout << "Unique names: " << name_to_position.size()
            << " interactions: " << interactions.size() << std::endl;
}

void Model::load_post_format(const string filename) {
  std::ifstream input(filename);
  string line;
  string word;
  int value;

  // TODO actually use the title line to set the variable names
  getline(input, line);
  getline(input, line);
  vector<int> settings, minimums;
  std::istringstream iss(line);

  while (iss >> value) {
    settings.push_back(value);
  }
  getline(input, line);
  iss.clear();
  iss.str(line);
  while (iss >> value) {
    minimums.push_back(value);
  }
  if (settings.size() != minimums.size()) {
    throw invalid_argument("Input file had mismatched ranges and minimums");
  }
  size_t index = 0;
  // For the rest of the lines
  while (getline(input, line)) {
    std::istringstream iss(line);
    Interaction interaction;
    iss >> interaction.target_name;
    // ignore the =
    iss >> word;
    if (word != "=") {
      throw invalid_argument("Input file missing = after interaction name");
    }
    string behavior;
    while (iss >> word >> behavior) {
      if (behavior == "PROMOTES") {
        interaction.activator_names.push_back(word);
      } else if (behavior == "INHIBITS") {
        interaction.inhibitor_names.push_back(word);
      } else {
        throw invalid_argument("Input file bad behavior for " + interaction.target_name + " of " + behavior);
      }
    }
    if (index >= minimums.size()) {
      throw invalid_argument("Input file had more interactions than bounds information");
    }
    interaction.lower_bound = minimums[index];
    interaction.upper_bound = minimums[index] + settings[index] - 1;
    interactions.push_back(interaction);
    index++;
  }
}

void Model::load_old_format(const string filename) {
  // TODO handle the fact that some states may not have an iteraction
  std::ifstream input(filename);
  string line;
  string word;
  while (getline(input, line)) {
    // Remove everything after the #
    auto result = line.find('#');
    line = line.substr(0, result);
    // Ignores lines that had nothing in them
    if (line.size() == 0) {
      continue;
    }
    Interaction interaction;
    std::istringstream iss(line);
    iss >> interaction.target_name;
    // Clear the first "|"
    iss >> word;
    if (word != "|") {
      throw invalid_argument("Bad input file had no '|' after the first variable name");
    }
    while (iss >> word and word != "|") {
      interaction.activator_names.push_back(word);
    }
    if (word != "|") {
      throw invalid_argument("Bad input file had no '|' after activators on line " + interaction.target_name);
    }
    while (iss >> word and word != "|") {
      interaction.inhibitor_names.push_back(word);
    }
    // check that iss is empty
    if (iss >> word) {
      throw invalid_argument("Bad input file had too many '|' symbols on line " + interaction.target_name);
    }
    // TODO do this for real
    interaction.lower_bound = -1;
    interaction.upper_bound = 1;
    // Add it to the list of interactions
    interactions.push_back(interaction);
  }
}

void Model::load_csv(const string filename) {
  std::ifstream input(filename);
  string line;
  int relationship;
  size_t row = 0;
  char clear_comma;
  while (getline(input, line)) {
    // Remove everything after the #
    auto result = line.find('#');
    line = line.substr(0, result);
    // Ignores lines that had nothing in them
    if (line.size() == 0) {
      continue;
    }
    // Create a new interaction
    Interaction interaction;
    interaction.target_name = to_string(row);
    // TODO do this for real
    interaction.lower_bound = -1;
    interaction.upper_bound = 1;
    row++;

    std::istringstream iss(line);
    size_t col = 0;
    while (iss >> relationship) {
      if (relationship == 1) {
        interaction.activator_names.push_back(to_string(col));
      } else if (relationship == -1) {
        interaction.inhibitor_names.push_back(to_string(col));
      }
      col++;
      iss >> clear_comma;
    }
    interactions.push_back(interaction);
  }
  for (auto & i : interactions) {
    if (i.activator_names.empty() and i.inhibitor_names.empty()) {
      i.lower_bound = 0;
      i.upper_bound = 0;
    }
  }
  /*
  for (const auto & i : interactions) {
    cout << i.target_name << " = ";
    for (const auto & a : i.activator_names) {
      cout << a << " PROMOTES ";
    }
    for (const auto & a : i.inhibitor_names) {
      cout << a << " INHIBITS ";
    }
    cout << endl;
  }
  //*/
}
#include <unordered_set>
using std::unordered_set;

void Model::reorganize() {
  // Maps subfunction to number of unset dependencies
  vector<unordered_set<int>> interaction_with_remaining(interactions.size() + 1, unordered_set<int>());
  // Location of a move in the move_bin
  vector<int> interaction_to_remaining(interactions.size(), -1);
  // Lookup for finding all moves with non-linear relationships to a bit
  unordered_map<string, vector<int>> name_to_interaction;

  // TODO combine with following loop
  vector<vector<string>> interaction_to_names;
  for (const auto & interaction : interactions) {
    unordered_set<string> names;
    names.insert(interaction.target_name);
    for (const auto name : interaction.activator_names) {
      names.insert(name);
    }
    for (const auto name : interaction.inhibitor_names) {
      names.insert(name);
    }
    interaction_to_names.emplace_back(names.begin(), names.end());
  }

  for (size_t i = 0; i < interactions.size(); i++) {
    size_t total = interaction_to_names[i].size();
    // Put the move in the bin based on its dependency
    interaction_with_remaining[total].insert(i);
    interaction_to_remaining[i] = total;
    // record all of its dependencies
    for (const auto name : interaction_to_names[i]) {
      name_to_interaction[name].push_back(i);
    }
  }

  // Initially no position is assigned to anything
  int highest_available = interactions.size() - 1;
  position_to_name.assign(interactions.size(), "");
  // Loop until every position is assigned
  while (highest_available >= 0) {
    // Find the move with the least remaining dependencies
    // that has yet to be assigned a position
    int i = -1;
    for (const auto& bin : interaction_with_remaining) {
      if (bin.size()) {
        i = *bin.begin();
        break;
      }
    }
    // If no more moves have to be assigned places, stop
    if (i == -1) {
      break;
    }
    // For all bits in all subfunctions related to that move,
    // assign those bits as high as possible
    for (const string name : interaction_to_names[i]) {
      // if the bit hasn't been assigned a new position
      auto result = name_to_position.insert({name, highest_available});
      if (result.second) {
        position_to_name[highest_available] = name;
        // Update how many dependencies all other moves have
        for (const auto affected : name_to_interaction[name]) {
          int current = interaction_to_remaining[affected];
          // shift the affected move down 1 in the sub_with_remaining
          interaction_with_remaining[current].erase(affected);
          interaction_with_remaining[current - 1].insert(affected);
          interaction_to_remaining[affected] = current - 1;
        }
        highest_available--;
      }
    }
    // Remove the assigned move from the move bin
    auto result = interaction_with_remaining[0].erase(i);
    if (result != 1) {
      cout << "Error: " << i << " not in 0 bucket" << endl;
      break;
    }
  }
}

size_t Model::find_position(const string& name) const {
  auto result = name_to_position.find(name);
  if (result == name_to_position.end()) {
    return -1;
  }
  return result->second;
}

void Model::print_header(std::ostream& out) const {
  for (const auto & column : original_ordering) {
    out << column << " ";
  }
  out << std::endl;
}

void Model::print(const vector<int>& current_state, std::ostream& out) const {
  // TODO update this if range increases
  const string lookup = "INA";
  for (size_t i=0; i < current_state.size(); i++) {
    string column = original_ordering[i];
    int value = current_state[name_to_position.at(column)];
    if (column == "CLOCK") {
      out << value << " ";
    } else {
      out << lookup[value + 1] << " ";
    }
  }
  out << std::endl;
}

void Model::print_tight(const vector<int>& current_state, std::ostream& out) const {
  // TODO update this if range increases
  const string lookup = "INA";
  for (size_t i=0; i < current_state.size(); i++) {
    string column = original_ordering[i];
    int value = current_state[name_to_position.at(column)];
    if (column == "CLOCK") {
      out << value - interactions[i].lower_bound;
    } else {
      out << lookup[value + 1];
    }
  }
}

vector<int> Model::load_state(string line) const {
  vector<int> result(size(), 0);
  unordered_map<string, int> lookup = {{"N", 0}, {"A", 1}, {"I", -1}};
  istringstream iss(line);
  int value;
  string symbol;
  for (size_t column=0; column < result.size(); column++) {
    string name = original_ordering[column];
    if (name == "CLOCK") {
      iss >> value;
    } else {
      iss >> symbol;
      value = lookup.at(symbol);
    }
    int position = name_to_position.at(name);
    result[position] = value;
  }

  return result;
}
