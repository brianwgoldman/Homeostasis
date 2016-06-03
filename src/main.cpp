// Brian Goldman

// ---------------- Compilation ----------------------------
// In Release/ and Debug/ there are makefiles to compile the source
// To perform experiments, call "make run" in the Release directory.
// For our experiments we used G++ 4.8.2.

// ---------------- Example Usage ----------------------------
// From the top level directory call:
// Release/run FOCUS.txt out.txt 0
// This will use the model from "FOCUS.txt" and write its output to "out.txt"
// performing operation "0", which corresponds to finding all stable states.

#include "Model.h"
#include "Enumeration.h"
#include "Cycles.h"
#include "MonteCarloCycles.h"
#include "WalkCycle.h"

#include <iostream>
using namespace std;
#include <cassert>
#include <fstream>

int main(int argc, char * argv[]) {
  if (argc < 3) {
    // Help message
    cout
        << "Usage: input_filename output_filename [tool]" << endl
        << endl
        << "Example: Release/run input.txt output.txt" << endl
        << "         This will read a problem from input.txt, write local optima to output.txt"
        << endl;
    return 0;
  }
  string problem_file = argv[1];
  string output_file = argv[2];
  int option = 0;
  if (argc > 3) {
    option = atoi(argv[3]);
  }

  // Start the timer
  auto start = std::chrono::steady_clock::now();
  // Read in the model
  Model model(problem_file);
  // Open the output file for writing
  ofstream out(output_file);
  if (option == 0) {
    cout << "You chose option 0: Finding all stable states" << endl;
    Enumeration enumerate(model);
    enumerate.enumerate(out);
  } else if (option == 1) {
    cout << "You chose option 1: Use synchronous updates and start from all states to see if they are cycles" << endl;
    Cycles cycle_finder(model);
    cycle_finder.find_cycles(out);
  } else if (option == 2) {
    cout << "You chose option 2: Using sampled Tarjan to find strongly connected components" << endl;
    Random random;
    MonteCarloCycles cycle_finder(model, random, 500000);
    for (int i=0; i < 100000; i++) {
      if (i % 1000 == 0) {
        cout << "Starting iteration: " << i << endl;
      }
      cycle_finder.iterate();
    }
    cycle_finder.print(out);
  } else if (option == 3) {
    cout << "You chose option 3: Perform random walks and record each type you get a cycle" << endl;
    Random random;
    random.seed(std::random_device()());
    WalkCycle cycle_finder(model, random, 500000);
    for (int i=0; i < 1000000; i++) {
      if (i % 1000 == 0) {
        cout << "Starting iteration: " << i << endl;
      }
      cycle_finder.iterate();
    }
    cycle_finder.print(out);
  } else if (option == 4) {
    if (argc < 5) {
      cout << "Option 4 requires another argument: the cycle input file" << endl;
      return 1;
    }
    cout << "You chose option 4: Convert a cycle into something that can be read by GraphViz" << endl;
    ifstream in(argv[4]);
    out << "digraph test {" << endl;
    out << "overlap=scalexy" << endl;
    string line;
    unordered_set<vector<int>> starts, ends;
    while (getline(in, line)) {
      vector<int> states = model.load_state(line);
      starts.insert(states);
      for (const auto & neighbor : model.get_clock_next_states(states)) {
        ends.insert(neighbor);
        model.print(states, out);
        out << " -> ";
        model.print(neighbor, out);
        out << ";" << endl;
      }
    }
    out << "}" << endl;
    cout << starts.size() << " " << ends.size() << endl;
  } else {
    cout << "You chose an option that doesn't exist: " << option << endl;
    return 1;
  }
  auto end = std::chrono::steady_clock::now();
  double seconds = std::chrono::duration<double>(end - start).count();
  out << "# Seconds: " << seconds << endl;
  cout << "Done. Total Seconds: " << seconds << endl;
  return 0;
}
