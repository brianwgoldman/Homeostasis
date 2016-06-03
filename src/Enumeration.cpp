// Brian Goldman

// Find all solutions of a given MK Landscape
// which cannot be improved by flipping "radius_"
// or less bits. This process is made much more efficient
// by exploiting features of the Gray-Box domain.

#include "Enumeration.h"
#include <algorithm>

using std::cout;
using std::endl;

Enumeration::Enumeration(const Model & model_)
    : model(model_),
      length(model_.size()),
      affects_of(length, std::unordered_set<size_t>()),
      index_needs_change(length, 0),
      target_needs_change(length, 0),
      total_changes_needed(0) {

  for (const auto & interaction : model.get_interactions()) {
    // Maps activators and inhibitors to the targets they affect
    for (const auto index : interaction.activators) {
      affects_of[index].insert(interaction.target);
    }
    for (const auto index : interaction.inhibitors) {
      affects_of[index].insert(interaction.target);
    }
    // A target changing effects itself
    affects_of[interaction.target].insert(interaction.target);
  }
}

void Enumeration::make_move(size_t index, int newstate) {
  reference[index] = newstate;
  for (const auto affected : affects_of[index]) {
    // Get get the interaction that needs to be updated now that "index" has changed
    const auto & interaction = model.get_interactions()[affected];
    if (affected != interaction.target) {
      throw std::invalid_argument(
          "Index mismatch between target and interaction number");
    }
    // check to see if the interaction is stable
    int desired = interaction.get_next_state(reference);
    int needs_change = desired != reference[affected];

    // 4 states:
    // * needs change now and did so before = 1 - 1 = 0
    // * needs change now and did not before = 1 - 0 = +1
    // * doesn't need change now, did before = 0 - 1 = -1
    // * doesn't need change now, didn't before = 0 - 0 = 0
    int delta = needs_change - target_needs_change[affected];
    index_needs_change[interaction.minimum_dependency] += delta;
    total_changes_needed += delta;
    target_needs_change[affected] = needs_change;
  }
}

size_t Enumeration::increment(size_t index) {
  const auto & interactions = model.get_interactions();
  // Perform carry operations
  while (reference[index] == interactions[index].upper_bound) {
    // reduce it from maximum to minimum
    make_move(index, interactions[index].lower_bound);
    index++;
    if (index >= length) {
      return index;
    }
  }
  // advance by 1
  make_move(index, reference[index] + 1);
  return index;
}

void Enumeration::rebuild_changes_needed() {
  // fill delta with 0s
  index_needs_change.assign(length, 0);
  target_needs_change.assign(length, 0);
  total_changes_needed = 0;
  for (const auto& interaction : model.get_interactions()) {
    int desired = interaction.get_next_state(reference);
    int needs_change = desired != reference[interaction.target];
    index_needs_change[interaction.minimum_dependency] += needs_change;
    target_needs_change[interaction.target] = needs_change;
    total_changes_needed += needs_change;
  }
}

void Enumeration::enumerate(std::ostream& out) {
  // start all variables at lower bound
  reference.resize(length);
  for (size_t i = 0; i < length; i++) {
    reference[i] = model.get_interactions()[i].lower_bound;
  }
  rebuild_changes_needed();

  // tracks how many stable states are found
  size_t count = 0;

  model.print_header(out);

  size_t index = length - 1;
  size_t iterations = 0;
  while (true) {
    iterations++;
    // If a local optima has been found, output it
    if (total_changes_needed == 0) {
      model.print(reference, out);
      count++;
    }
    // Hyperplanes let you skip areas below the highest
    // non-zero move bin
    while (index > 0 and index_needs_change[index] == 0) {
      index--;
    }
    // increment that index
    index = increment(index);
    // End is reached
    if (index >= length) {
      cout << endl;
      out << "# Count: " << count << endl;
      cout << "Count: " << count << endl;
      return;
    }

    // Everything below here is just for screen output purposes
    if (iterations % 500000 == 0) {
      size_t start = 0;
      if (reference.size() > 150) {
        start = reference.size() - 150;
      }
      string magic = "-*#";
      for (size_t i = start; i < reference.size(); i++) {
        cout << magic[reference[i] + 1];
      }
      cout << endl;
    }
  }
}
