#include "megasampler.h"
#include "pythoncaller.h"
#include <iostream>

MEGASampler::MEGASampler(std::string input, int max_samples, double max_time,
                         int max_epoch_samples, double max_epoch_time,
                         int strategy)
    : Sampler(input, max_samples, max_time, max_epoch_samples, max_epoch_time,
              strategy),
      simpl_formula(c) {
  initialize_solvers();
  std::cout << "starting MEGA" << std::endl;
}

void MEGASampler::do_epoch(const z3::model &model) {
  call_strengthen(original_formula, model);
}
