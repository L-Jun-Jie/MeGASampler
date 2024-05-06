
#include "intervalmap.h"

bool is_inf(const IntervalMap& i_map) {
  for (const auto& varinterval : i_map) {
    const auto& interval = varinterval.second;
    if (interval.is_infinite()) return true;
  }
  return false;
}

/**
 * Returns false if the interval is infinite, 
 * true if it is finite, 
 * and the size of the interval is returned by reference i_size
*/
bool intervals_size(const IntervalMap& i_map, int64_t& i_size) {
  int64_t size = 1;
  for (const auto& varinterval : i_map) {
    const auto& interval = varinterval.second;
    if (interval.is_infinite()) return false;
    size *= (interval.get_high() - interval.get_low()) + 1;  // TODO: safe_mul
  }
  i_size = size;
  return true;
}
