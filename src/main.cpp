// Brian Goldman

// Parse arguments and drive the enumeration tool.
// Will read in an MK Landscape from a specified file and then
// write all of its r-bit local optima to another specified file.
// An r-bit local optima is a solution which cannot be improved
// by flipping r or less bits.

// ---------------- Compilation ----------------------------
// In Release/ and Debug/ there are makefiles to compile the source
// To perform experiments, call "make MKL" in the Release directory.
// For our experiments we used G++ 4.8.2.

// ---------------- Example Usage ----------------------------
// From the top level directory call:
// Release/MKL problem_folder/DeceptiveTrap_20_5_0.txt output.txt 1
// This will find all local optima of a Deceptive Trap problem using
// N=20 and k=5 and random seed 0 (shuffled varible ordering) as the landscape
// and will output all 1 bit local optima to "output.txt"
// There are also optional switches which can disable hyperplane elimination and
// disable reordering, both of which are enabled by default.

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

  auto start = std::chrono::steady_clock::now();
  Model model(problem_file);
  ofstream out(output_file);
  if (option == 0) {
    Enumeration enumerate(model);

    enumerate.enumerate(out);
  } else if (option == 1) {
    Cycles cycle_finder(model);
    cycle_finder.find_cycles(out);
  } else if (option == 2) {
    Random random;
    MonteCarloCycles cycle_finder(model, random, 500000);
    for (int i=0; i < 1000000; i++) {
      if (i % 1000 == 0) {
        cout << "Starting iteration: " << i << endl;
      }
      cycle_finder.iterate();
    }
    cycle_finder.print(out);
  } else if (option == 3) {

    Random random;
    random.seed(std::random_device()());
    WalkCycle cycle_finder(model, random, 500000);
    for (int i=0; i < 4000000; i++) {
      if (i % 1000 == 0) {
        cout << "Starting iteration: " << i << endl;
      }
      cycle_finder.iterate();
    }
    cycle_finder.print(out);
  }else {
    cout << "Bad option!" << endl;
  }
  auto end = std::chrono::steady_clock::now();
  double seconds = std::chrono::duration<double>(end - start).count();
  out << "# Seconds: " << seconds << endl;
  cout << "Done. Total Seconds: " << seconds << endl;
  return 0;
}
