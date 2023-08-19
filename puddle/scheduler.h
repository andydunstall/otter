#pragma once

#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/scheduler.hpp>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace puddle {

class Shard;

// Scheduler is a custom fiber scheduling algorithm.
//
// The algorithm is uses round-robin scheduling, similar to boost round_robin,
// except suspend_until will poll for IO instead of sleeping.
class Scheduler : public boost::fibers::algo::algorithm {
 public:
  Scheduler(std::shared_ptr<Shard> shard);

  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  void awakened(boost::fibers::context* ctx) noexcept override;

  boost::fibers::context* pick_next() noexcept override;

  bool has_ready_fibers() const noexcept override;

  void suspend_until(const std::chrono::steady_clock::time_point&
                         time_point) noexcept override;

  void notify() noexcept override;

 private:
  std::shared_ptr<Shard> shard_;

  boost::fibers::scheduler::ready_queue_type rqueue_{};

  std::mutex mtx_{};

  std::condition_variable cnd_{};
};

}  // namespace puddle
