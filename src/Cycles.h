// Brian Goldman

// Performs an exhaustive search for cycles when
// using synchronous updating.
#ifndef CYCLES_H_
#define CYCLES_H_

#include "Model.h"

class Cycles {
 public:
  Cycles(const Model& model_);
  // Will write cycles to "out" as they are found, stopping once the space is exhausted.
  void find_cycles(std::ostream& out);
 private:
  const Model& model;
  // Given a list of states, find the next lowest state position.
  // If called repeatedly starting from all states at their lower_bounds,
  // this will visit all states once before returning false.
  bool increment(vector<int> & current) const;
};

#endif /* CYCLES_H_ */
