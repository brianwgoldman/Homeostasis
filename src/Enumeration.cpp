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
      target_needs_change(length, 0){

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
    const auto & interaction = model.get_interactions()[affected];
    if (affected != interaction.target) {
      std::invalid_argument("Index mismatch between target and interaction number");
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
  const auto & bounds = model.get_bounds();
  // Perform carry operations
  while (reference[index] == bounds[index].second) {
    // reduce it from maximum to minimum
    make_move(index, bounds[index].first);
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
    target_needs_change[interaction.target] += needs_change;
    total_changes_needed += needs_change;
  }
}

void Enumeration::enumerate(std::ostream& out) {
  // start from all 0s
  reference.resize(length);
  for (size_t i=0; i < length; i++) {
    // set it to the minimum bound
    reference[i] = model.get_bounds()[i].first;
  }
  rebuild_changes_needed();

  // tracks how many local optima are found
  size_t count = 0;

  // Used to output progress to the screen
  int pass = 1;
  size_t milestone = 0;
  cout << "Pass " << pass << ": ";
  model.print_header(out);

  size_t index = length - 1;
  while (true) {
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
    if (index >= milestone) {
      milestone = index + 1;
      cout << index << ", ";
      cout.flush();
      if (milestone > length - pass) {
        milestone = 0;
        cout << endl << "Pass " << pass << ": ";
        pass++;
      }
    }
  }
}

