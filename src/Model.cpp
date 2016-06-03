// Brian Goldman
// Provides an implementation of trinary logic, asynchronous and synchronous model
// updates, and file-io.
#include "Model.h"
#include <algorithm>
using std::max_element;
#include <sstream>
using std::endl;
#include <exception>
using std::invalid_argument;
using std::to_string;
#include <unordered_set>
using std::unordered_set;
#include <cassert>
#include <sstream>
using std::istringstream;
#include <unordered_set>
using std::unordered_set;

int Interaction::get_direction_of_change(
    const vector<int>& current_states) const {
  vector<int> activator_states;
  // TODO Currently this treats all values > 0 as activated and all values < 0 as inhibited
  // you should probably allow each variable to control that information.
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
      // If you have neither activators or inhibitors, you just keep your state
      return current_states[target];
    }
    return *std::max_element(activator_states.begin(), activator_states.end());

  // Line 3 in Equation 2
  } else if (activators.size() == 0) {
    auto result = *std::max_element(inhibitor_states.begin(),
                                    inhibitor_states.end());
    // Perform negation
    return -result;
  // Line 1 in Equation 2
  } else {
    auto active_aggregate = *std::max_element(activator_states.begin(),
                                              activator_states.end());
    auto inhibit_aggregate = *std::max_element(inhibitor_states.begin(),
                                               inhibitor_states.end());
    // Activated is > 0
    if (active_aggregate > 0 and inhibit_aggregate <= 0) {
      return active_aggregate;
    } else if (inhibit_aggregate > 0 and active_aggregate <= 0) {
      return -inhibit_aggregate;
    } else {
      return 0;
    }
  }
}

int Interaction::get_next_state(const vector<int>& current_states) const {
  int delta = get_direction_of_change(current_states);
  int next_state = current_states[target];
  // This handles the "gradual change" idea
  if (delta > 0) {
    // Try to become more active
    if (next_state < upper_bound) {
      next_state++;
    }
  } else if (delta < 0) {
    // Try to become more less active
    if (next_state > lower_bound) {
      next_state--;
    }
  } else {
    // Neutral falls towards 0
    if (next_state > 0) {
      next_state--;
    } else if (next_state) {
      next_state++;
    }
  }
  return next_state;
}

vector<int> Model::get_sync_next(const vector<int>& current_states) const {
  vector<int> result(current_states);
  for (const auto & interaction : interactions) {
    // Determine how this interaction wants to change
    result[interaction.target] = interaction.get_next_state(current_states);
  }
  return result;
}

vector<vector<int>> Model::get_async_next_states(
    const vector<int>& current_states) const {
  vector<vector<int>> result;
  for (const auto & interaction : interactions) {
    int next_state = interaction.get_next_state(current_states);
    // If a change is desired, create a new option where only "target" is changed.
    if (next_state != current_states[interaction.target]) {
      result.push_back(current_states);
      result.back()[interaction.target] = next_state;
    }
  }
  return result;
}

vector<vector<int>> Model::get_clock_next_states(
    const vector<int>& current_states) const {
  // This is the brain
  const unordered_set<string> brain = { "LH/FSH", "ACTH", "GnRH", "CRH" };
  // TODO This should probably be moved out of this function and made a part of model
  // Find the CLOCK
  size_t clock = interactions.size();
  bool brain_phase = false;
  for (const auto& interaction : interactions) {
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
    // Determine if this is a brain interaction or not
    bool is_brain = brain.count(interaction.target_name) == 1;
    int next_state = interaction.get_next_state(current_states);

    if (next_state != current_states[interaction.target]) {
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
  for (size_t i = 0; i < interactions.size(); i++) {
    const auto & interaction = interactions[i];
    // generate a random number between lower_bound and upper_bound (inclusive)
    std::uniform_int_distribution<int> dist(interaction.lower_bound,
                                            interaction.upper_bound);
    result[interaction.target] = dist(random);
  }
  return result;
}

Model::Model(string filename) {
  if (filename.substr(filename.size() - 3, 3) == "csv") {
    std::cout << "Loading CSV" << endl;
    load_csv(filename);
  } else {
    load_post_format(filename);
  }

  position_to_name.resize(interactions.size());
  original_ordering.resize(interactions.size());

  // Assign names to initial numeric positions
  for (size_t i = 0; i < interactions.size(); i++) {
    auto name = interactions[i].target_name;
    original_ordering[i] = name;
  }
  // Change the ordering to make everything better for enumeration
  reorganize();

  // Set up the numeric positions of "target", "activators", "inhibitors", and "minimum_dependency"
  for (auto & interaction : interactions) {
    interaction.target = name_to_position[interaction.target_name];
    size_t min_dep = interaction.target;
    for (const auto name : interaction.activator_names) {
      auto result = name_to_position.find(name);
      if (result == name_to_position.end()) {
        throw invalid_argument(
            "Input file had " + interaction.target_name + " have activator "
                + name + " that has no line of its own.");
      }
      interaction.activators.push_back(result->second);
      min_dep = std::min(min_dep, interaction.activators.back());
    }
    for (const auto name : interaction.inhibitor_names) {
      auto result = name_to_position.find(name);
      if (result == name_to_position.end()) {
        throw invalid_argument(
            "Input file had " + interaction.target_name + " have inhibitor "
                + name + " that has no line of its own.");
      }
      interaction.inhibitors.push_back(result->second);
      min_dep = std::min(min_dep, interaction.inhibitors.back());
    }
    interaction.minimum_dependency = min_dep;
  }
  // Put interactions into order based on their target
  size_t index = 0;
  while (index < interactions.size()) {
    // Put whatever is at "index" into the position it wants to go to
    std::swap(interactions[index], interactions[interactions[index].target]);
    // If "index" contains the correct thing, advance
    if (index == interactions[index].target) {
      index++;
    }
  }

  std::cout << "Unique names: " << name_to_position.size() << " interactions: "
            << interactions.size() << std::endl;
}

void Model::load_post_format(const string filename) {
  std::ifstream input(filename);
  string line;
  string word;
  int value;

  // Read the top line to find all of the names
  getline(input, line);
  vector<string> header_names;
  std::istringstream iss(line);
  while (iss >> word) {
    header_names.push_back(word);
  }

  // Read the second line to get the range of each variable
  getline(input, line);
  vector<int> settings;
  iss.clear();
  iss.str(line);
  while (iss >> value) {
    settings.push_back(value);
  }

  // Read the third line to get the minimum value of each variable
  getline(input, line);
  iss.clear();
  iss.str(line);
  vector<int> minimums;
  while (iss >> value) {
    minimums.push_back(value);
  }
  if (header_names.size() != settings.size()) {
    throw invalid_argument("Input file had mismatched header names and ranges");
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
    if (interaction.target_name == "") {
      // Skip blank lines
      continue;
    }
    if (index >= header_names.size()) {
      throw invalid_argument(
          "Input file had more rows than names in the header/ranges for variables");
    }
    if (interaction.target_name != header_names[index]) {
      throw invalid_argument(
          "Input file had rows in a different order than header names");
    }
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
        throw invalid_argument(
            "Input file bad behavior for " + interaction.target_name + " of "
                + behavior);
      }
    }
    interaction.lower_bound = minimums[index];
    interaction.upper_bound = minimums[index] + settings[index] - 1;
    interactions.push_back(interaction);
    index++;
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
}

void Model::reorganize() {
  // interaction_with_remaining[X] stores the set of "interaction"s with X dependencies that don't
  // have positions yet
  vector<unordered_set<int>> interaction_with_remaining(interactions.size() + 1,
                                                        unordered_set<int>());
  // interaction_to_remaining[i] gives you where "i" appears in "interactions_with_remaining"
  vector<int> interaction_to_remaining(interactions.size(), -1);
  // Lookup for finding all "interactions" that depend on a given variable
  unordered_map<string, vector<int>> name_to_interaction;

  // Stores the list of names that interaction[i] interacts with.
  vector<vector<string>> interaction_to_names;
  for (const auto & interaction : interactions) {
    // Combine the names of yourself, your activators, and your inhibitors
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
    // Put the interaction in the bin based on how many things it depends on
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
    // Find the interaction with the least remaining dependencies
    // that has yet to be assigned a position
    int i = -1;
    for (const auto& bin : interaction_with_remaining) {
      if (bin.size()) {
        i = *bin.begin();
        break;
      }
    }
    // If no more interactions have to be assigned places, stop
    if (i == -1) {
      break;
    }
    // Assign all names used by interaction[i] to the highest remaining positions
    for (const string name : interaction_to_names[i]) {
      // if the bit hasn't been assigned a new position
      auto result = name_to_position.insert( { name, highest_available });
      if (result.second) {
        position_to_name[highest_available] = name;
        // Update how many dependencies all other interactions have
        for (const auto affected : name_to_interaction[name]) {
          int current = interaction_to_remaining[affected];
          // shift the affected interaction down 1
          interaction_with_remaining[current].erase(affected);
          interaction_with_remaining[current - 1].insert(affected);
          interaction_to_remaining[affected] = current - 1;
        }
        highest_available--;
      }
    }
    // Remove the assigned interaction from the interaction_with_remaining
    auto result = interaction_with_remaining[0].erase(i);
    assert(result == 1);
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
  for (size_t i = 0; i < current_state.size(); i++) {
    string column = original_ordering[i];
    int value = current_state[name_to_position.at(column)];
    if (value >= 0) {
      out << " ";
    }
    out << value << " ";
  }
  out << std::endl;
}

vector<int> Model::load_state(string line) const {
  vector<int> result(size(), 0);
  istringstream iss(line);
  int value;
  for (size_t column = 0; column < result.size(); column++) {
    string name = original_ordering[column];
    iss >> value;
    int position = name_to_position.at(name);
    result[position] = value;
  }

  return result;
}
