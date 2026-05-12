#ifndef CURRENT_MONITOR_UTILS_H
#define CURRENT_MONITOR_UTILS_H

#include <math.h>

static inline float compute_rms(const int *samples, int count, int offset) {
  if (count <= 0)
    return 0.0f;
  double sum_sq = 0;
  for (int i = 0; i < count; i++) {
    float centered = (float)samples[i] - (float)offset;
    sum_sq += (centered * centered);
  }
  return sqrt(sum_sq / count);
}

#endif // CURRENT_MONITOR_UTILS_H
