#pragma once

#include <cstdint>
#include <vector>

namespace puddle {
namespace stats {

class Histogram {
 public:
  uint64_t Percentile(double p) const;

  double StdDev() const;

  uint64_t min() const { return min_; }

  uint64_t max() const { return max_; }

  void Add(uint64_t value) { Add(value, 1); };

  void Add(uint64_t value, uint64_t count);

  void Merge(const Histogram& h);

 private:
  static constexpr int kNumBuckets = 154;
  static const double kBucketLimit[kNumBuckets];

  double InterpolateVal(uint64_t bucket, uint64_t position) const;

  std::pair<double, double> BucketLimits(uint64_t b) const;

  uint64_t min_ = UINT64_MAX;

  uint64_t max_ = 0;

  uint64_t num_ = 0;

  uint64_t sum_ = 0;

  uint64_t sum_squares_ = 0;

  std::vector<uint64_t> buckets_;
};

}  // namespace stats
}  // namespace puddle
