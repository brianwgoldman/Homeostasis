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

STATE Interaction::get_next_state(const vector<STATE>& current_states) const {
  vector<STATE> activator_states;
  for (const auto index : activators) {
    activator_states.push_back(current_states[index]);
  }
  vector<STATE> inhibitor_states;
  for (const auto index : inhibitors) {
    inhibitor_states.push_back(current_states[index]);
  }
  // Line 2 in Equation 2
  if (inhibitors.size() == 0) {
    return *std::max_element(activator_states.begin(), activator_states.end());

  // Line 3 in Equation 2
  } else if (activators.size() == 0) {
    auto result = *std::max_element(inhibitor_states.begin(), inhibitor_states.end());
    // Perform negation
    if (result == INHIBITED) {
      result = ACTIVATED;
    } else if (result == ACTIVATED) {
      result = INHIBITED;
    }
    return result;
  // Line 1 in Equation 2
  } else {
    auto active_aggregate = *std::max_element(activator_states.begin(), activator_states.end());
    auto inhibit_aggregate = *std::max_element(inhibitor_states.begin(), inhibitor_states.end());
    if (active_aggregate == ACTIVATED and inhibit_aggregate != ACTIVATED) {
      return ACTIVATED;
    } else if (inhibit_aggregate == ACTIVATED and active_aggregate != ACTIVATED) {
      return INHIBITED;
    } else {
      return NOMINAL;
    }
  }
}

Model::Model(string filename) {
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
    // Add it to the list of interactions
    interactions.push_back(interaction);
  }
  position_to_name.resize(interactions.size());
  original_ordering.resize(interactions.size());

  // Assign names to numeric positions
  for (size_t i=0; i < interactions.size(); i++) {
    auto name = interactions[i].target_name;
    original_ordering[i] = name;
    position_to_name[i] = name;
    auto result = name_to_position.insert({name, i});
    if (not result.second) {
      throw invalid_argument("Input file had two lines for the same target: " + name);
    }
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

  std::cout << "Unique names: " << name_to_position.size()
            << " interactions: " << interactions.size() << std::endl;
}

void Model::print_header(std::ostream& out) const {
  for (const auto & column : original_ordering) {
    out << column << " ";
  }
  out << std::endl;
}

void Model::print(const vector<STATE>& current_state, std::ostream& out) const {
  const string lookup = "INA";
  for (size_t i=0; i < current_state.size(); i++) {
    string column = original_ordering[i];
    STATE value = current_state[name_to_position.at(column)];
    out << lookup[value] << " ";
  }
  out << std::endl;
}
