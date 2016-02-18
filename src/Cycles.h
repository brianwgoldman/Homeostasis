/*
 * Cycles.h
 *
 *  Created on: Jan 27, 2016
 *      Author: goldman
 */

#ifndef CYCLES_H_
#define CYCLES_H_

#include "Model.h"

class Cycles {
 public:
  Cycles(const Model& model_);
  void find_cycles(std::ostream& out);
 private:
  const Model& model;
  bool increment(vector<int> & current) const;
};

#endif /* CYCLES_H_ */
