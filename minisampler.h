#ifndef MINISAMPLER_H_
#define MINISAMPLER_H_
#include "sampler.h"

class MiniSampler : public Sampler {
 public:
  MiniSampler(z3::context* c, const std::string& input,
                           const std::string &output,
                           const MeGA::SamplerConfig &config)
    : Sampler(c, input, output, config) {
    initialize_solvers();
  }
  void do_epoch(__attribute__((unused)) const z3::model &m) {
    /* do nothing, all the work was done in Sampler::start_epoch */
  }

  void finish() {
    json_output["method name"] = "Z3";
    Sampler::finish();
  }
};

#endif /* MINISAMPLER_H_ */
