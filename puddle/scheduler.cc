#include "puddle/scheduler.h"

#include <boost/context/detail/prefetch.hpp>

#include "absl/log/check.h"
#include "puddle/shard.h"

namespace puddle {

Scheduler::Scheduler(std::shared_ptr<Shard> shard) : shard_{shard} {}

void Scheduler::awakened(boost::fibers::context* ctx) noexcept {
  CHECK(nullptr != ctx);
  CHECK(!ctx->ready_is_linked());
  CHECK(ctx->is_resumable());
  ctx->ready_link(rqueue_);
}

boost::fibers::context* Scheduler::pick_next() noexcept {
  boost::fibers::context* next = nullptr;
  if (!rqueue_.empty()) {
    next = &rqueue_.front();
    rqueue_.pop_front();
    boost::context::detail::prefetch_range(next,
                                           sizeof(boost::fibers::context));
    CHECK(nullptr != next);
    CHECK(!next->ready_is_linked());
    CHECK(next->is_resumable());
  }
  return next;
}

bool Scheduler::has_ready_fibers() const noexcept { return !rqueue_.empty(); }

void Scheduler::suspend_until(
    const std::chrono::steady_clock::time_point& time_point) noexcept {
  if (std::chrono::steady_clock::time_point::max() == time_point) {
    // Poll for IO until explicitly awoken.
    shard_->Poll(-1);
  } else {
    auto d = time_point - std::chrono::steady_clock::now();
    if (d.count() > 0) {
      shard_->Poll(
          std::chrono::duration_cast<std::chrono::milliseconds>(d).count());
    } else {
      // If the time point is in the past don't block.
      shard_->Poll(0);
    }
  }
}

void Scheduler::notify() noexcept { shard_->Wake(); }

}  // namespace puddle
