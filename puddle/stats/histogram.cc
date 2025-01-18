#include "puddle/stats/histogram.h"

#include <algorithm>
#include <cmath>

#include "absl/numeric/bits.h"

namespace puddle {
namespace stats {

const double Histogram::kBucketLimit[kNumBuckets] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    12,
    14,
    16,
    18,
    20,
    25,
    30,
    35,
    40,
    45,
    50,
    60,
    70,
    80,
    90,
    100,
    120,
    140,
    160,
    180,
    200,
    250,
    300,
    350,
    400,
    450,
    500,
    600,
    700,
    800,
    900,
    1000,
    1200,
    1400,
    1600,
    1800,
    2000,
    2500,
    3000,
    3500,
    4000,
    4500,
    5000,
    6000,
    7000,
    8000,
    9000,
    10000,
    12000,
    14000,
    16000,
    18000,
    20000,
    25000,
    30000,
    35000,
    40000,
    45000,
    50000,
    60000,
    70000,
    80000,
    90000,
    100000,
    120000,
    140000,
    160000,
    180000,
    200000,
    250000,
    300000,
    350000,
    400000,
    450000,
    500000,
    600000,
    700000,
    800000,
    900000,
    1000000,
    1200000,
    1400000,
    1600000,
    1800000,
    2000000,
    2500000,
    3000000,
    3500000,
    4000000,
    4500000,
    5000000,
    6000000,
    7000000,
    8000000,
    9000000,
    10000000,
    12000000,
    14000000,
    16000000,
    18000000,
    20000000,
    25000000,
    30000000,
    35000000,
    40000000,
    45000000,
    50000000,
    60000000,
    70000000,
    80000000,
    90000000,
    100000000,
    120000000,
    140000000,
    160000000,
    180000000,
    200000000,
    250000000,
    300000000,
    350000000,
    400000000,
    450000000,
    500000000,
    600000000,
    700000000,
    800000000,
    900000000,
    1000000000,
    1200000000,
    1400000000,
    1600000000,
    1800000000,
    2000000000,
    2500000000.0,
    3000000000.0,
    3500000000.0,
    4000000000.0,
    4500000000.0,
    5000000000.0,
    6000000000.0,
    7000000000.0,
    8000000000.0,
    9000000000.0,
    1e200,
};

uint64_t Histogram::Percentile(double p) const {
  uint64_t threshold = num_ * (p / 100.0);
  uint64_t sum = 0;
  for (uint64_t b = 0; b < buckets_.size(); b++) {
    sum += buckets_[b];
    if (sum >= threshold) {
      // Scale linearly within this bucket
      uint64_t left_sum = sum - buckets_[b];
      return InterpolateVal(b, threshold - left_sum);
    }
  }
  return max_;
}

double Histogram::StdDev() const {
  if (num_ == 0) {
    return 0;
  }

  double variance = (sum_squares_ * num_ - sum_ * sum_) / (num_ * num_);
  return std::sqrt(variance);
}

void Histogram::Add(uint64_t value, uint64_t count) {
  auto it =
      std::upper_bound(kBucketLimit, kBucketLimit + kNumBuckets - 1, value);
  size_t b = it - kBucketLimit;
  if (buckets_.size() <= b) {
    buckets_.resize(absl::bit_ceil(b + 1));
  }
  buckets_[b] += count;

  if (value < min_) {
    min_ = value;
  }
  if (value > max_) {
    max_ = value;
  }
  num_++;
  sum_ += value * count;
  sum_squares_ += (value * value) * count;
}

void Histogram::Merge(const Histogram& h) {
  if (h.buckets_.size() > buckets_.size()) {
    buckets_.resize(h.buckets_.size());
  }
  for (uint64_t b = 0; b < h.buckets_.size(); b++) {
    buckets_[b] += h.buckets_[b];
  }

  if (h.min_ < min_) {
    min_ = h.min_;
  }
  if (h.max_ > max_) {
    max_ = h.max_;
  }
  num_ += h.num_;
  sum_ += h.sum_;
  sum_squares_ += h.sum_squares_;
}

double Histogram::InterpolateVal(uint64_t bucket, uint64_t position) const {
  auto limits = BucketLimits(bucket);

  // We divide by n+1 according to
  // http://en.wikipedia.org/wiki/Uniform_distribution_(continuous)#Order_statistics
  double pos = double(position) / double(buckets_[bucket] + 1);
  double r = limits.first + (limits.second - limits.first) * pos;
  if (r < min_) r = min_;
  if (r > max_) r = max_;
  return r;
}

std::pair<double, double> Histogram::BucketLimits(uint64_t b) const {
  double low = (b == 0) ? 0.0 : kBucketLimit[b - 1];
  return std::pair<double, double>(low, kBucketLimit[b]);
}

}  // namespace stats
}  // namespace puddle
