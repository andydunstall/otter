#pragma once

#include <chrono>

#include "boost/context/fiber.hpp"
#include "boost/intrusive/list.hpp"
#include "boost/intrusive/set.hpp"
#include "boost/intrusive_ptr.hpp"
#include "puddle/internal/sync.h"

namespace puddle {
namespace internal {

class Reactor;
class Scheduler;

using ReadyHook = boost::intrusive::list_member_hook<
    boost::intrusive::link_mode<boost::intrusive::safe_link>>;

using SleepHook = boost::intrusive::set_member_hook<
    boost::intrusive::link_mode<boost::intrusive::safe_link>>;

// Context represents the a tasks execution state.
class Context {
 public:
  Context();

  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;

  void Join();

  void Suspend();

  // Schedule adds the context to the ready queue.
  void Schedule();

  friend void intrusive_ptr_add_ref(Context* c) noexcept;
  friend void intrusive_ptr_release(Context* c) noexcept;

 protected:
  static constexpr size_t kStackSize = 64 * 1024;

  // Required for access to context_.
  friend Reactor;

  boost::context::fiber Terminate();

  // Holds the tasks execution context when the task is not active.
  boost::context::fiber context_;

 private:
  // Required to access intrusive member hooks.
  friend Scheduler;

  ReadyHook ready_hook_;

  SleepHook sleep_hook_;

  // Time the context is asleep till when it is in the schedulers sleep queue.
  std::chrono::steady_clock::time_point sleep_tp_;

  // Queue of contexts waiting for this context to terminate.
  WaitQueue join_queue_;

  bool terminated_;

  // Reference counter for intrusive_ptr.
  size_t ref_count_;
};

inline void intrusive_ptr_add_ref(Context* c) noexcept {
  // TODO(andydunstall)
}

inline void intrusive_ptr_release(Context* c) noexcept {
  // TODO(andydunstall)
}

// TaskContext is a context to run the given function.
template <typename Fn, typename... Arg>
class TaskContext final : public Context {
 public:
  TaskContext(const boost::context::preallocated& palloc,
              boost::context::fixedsize_stack salloc, Fn&& fn, Arg... arg);

  // Allocates a task context and stack.
  static boost::intrusive_ptr<Context> Spawn(Fn&& fn, Arg&&... arg);

 private:
  boost::context::fiber Run(boost::context::fiber&& c);

  typename std::decay<Fn>::type fn_;

  std::tuple<Arg...> arg_;
};

template <typename Fn, typename... Arg>
TaskContext<Fn, Arg...>::TaskContext(const boost::context::preallocated& palloc,
                                     boost::context::fixedsize_stack salloc,
                                     Fn&& fn, Arg... arg)
    : Context{}, fn_(std::forward<Fn>(fn)), arg_(std::forward<Arg>(arg)...) {
  context_ = boost::context::fiber{
      std::allocator_arg, palloc, salloc,
      std::bind(&TaskContext::Run, this, std::placeholders::_1)};
}

template <typename Fn, typename... Arg>
boost::intrusive_ptr<Context> TaskContext<Fn, Arg...>::Spawn(Fn&& fn,
                                                             Arg&&... arg) {
  // Allocate a stack for the context, then add the context structure at the
  // start of the stack.
  //
  // Copied from boost fibers make_worker_context_with_properties.

  typedef TaskContext<Fn, Arg...> context_t;

  boost::context::fixedsize_stack salloc{Context::kStackSize};
  auto sctx = salloc.allocate();

  void* storage =
      reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(sctx.sp) -
                               static_cast<uintptr_t>(sizeof(context_t))) &
                              ~static_cast<uintptr_t>(0xff));
  void* stack_bottom = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
  const std::size_t size = reinterpret_cast<uintptr_t>(storage) -
                           reinterpret_cast<uintptr_t>(stack_bottom);
  return boost::intrusive_ptr<Context>{new (storage) context_t{
      boost::context::preallocated{storage, size, sctx}, salloc,
      std::forward<Fn>(fn), std::forward<Arg>(arg)...}};
}

template <typename Fn, typename... Arg>
boost::context::fiber TaskContext<Fn, Arg...>::Run(boost::context::fiber&& c) {
  // Run the user function then terminate the context.
  auto fn = std::move(fn_);
  auto arg = std::move(arg_);
  std::apply(std::move(fn), std::move(arg));
  return Terminate();
}

}  // namespace internal
}  // namespace puddle
