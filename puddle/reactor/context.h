#pragma once

#include <atomic>

#include "boost/context/fiber.hpp"
#include "boost/intrusive_ptr.hpp"

namespace puddle {
namespace reactor {

class Reactor;

namespace internal {

constexpr size_t kStackSize = 64 * 1024;

// Context represents the a tasks execution state.
class Context {
 public:
  Context(const std::string& name);

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;

  const std::string& name() const { return name_; }

  void Join();

  friend void intrusive_ptr_add_ref(Context* c) noexcept;
  friend void intrusive_ptr_release(Context* c) noexcept;

 protected:
  friend Reactor;

  boost::context::fiber Terminate();

  // Holds the tasks execution context when the task is not active.
  boost::context::fiber context_;

 private:
  std::string name_;

  // TODO(andydunstall): Temporary.
  Context* join_wait_;

  bool terminated_;

  // Reference counter for intrusive_ptr.
  std::atomic<size_t> ref_count_;
};

inline void intrusive_ptr_add_ref(Context* c) noexcept {
  assert(c != nullptr);
  c->ref_count_.fetch_add(1, std::memory_order_relaxed);
}

inline void intrusive_ptr_release(Context* c) noexcept {
  assert(c != nullptr);
  if (c->ref_count_.fetch_sub(1, std::memory_order_release) == 1) {
    std::atomic_thread_fence(std::memory_order_acquire);

    // TODO(andydunstall): Need to understand this better.
    boost::context::fiber context = std::move(c->context_);
    c->~Context();
    std::move(context).resume();
  }
}

// TaskContext is a context to run the given function.
template <typename Fn, typename... Arg>
class TaskContext final : public Context {
 public:
  TaskContext(const std::string& name,
              const boost::context::preallocated& palloc,
              boost::context::fixedsize_stack salloc, Fn&& fn, Arg... arg);

  // Allocates a task context and stack.
  static boost::intrusive_ptr<Context> Allocate(const std::string& name,
                                                Fn&& fn, Arg&&... arg);

 private:
  boost::context::fiber Run(boost::context::fiber&& c);

  typename std::decay<Fn>::type fn_;

  std::tuple<Arg...> arg_;
};

template <typename Fn, typename... Arg>
TaskContext<Fn, Arg...>::TaskContext(const std::string& name,
                                     const boost::context::preallocated& palloc,
                                     boost::context::fixedsize_stack salloc,
                                     Fn&& fn, Arg... arg)
    : Context{name},
      fn_(std::forward<Fn>(fn)),
      arg_(std::forward<Arg>(arg)...) {
  context_ = boost::context::fiber{
      std::allocator_arg, palloc, salloc,
      std::bind(&TaskContext::Run, this, std::placeholders::_1)};
}

template <typename Fn, typename... Arg>
boost::intrusive_ptr<Context> TaskContext<Fn, Arg...>::Allocate(
    const std::string& name, Fn&& fn, Arg&&... arg) {
  // Copied from boost fibers make_worker_context_with_properties.

  typedef TaskContext<Fn, Arg...> context_t;

  boost::context::fixedsize_stack salloc{kStackSize};
  auto sctx = salloc.allocate();

  void* storage =
      reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(sctx.sp) -
                               static_cast<uintptr_t>(sizeof(context_t))) &
                              ~static_cast<uintptr_t>(0xff));
  void* stack_bottom = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
  const std::size_t size = reinterpret_cast<uintptr_t>(storage) -
                           reinterpret_cast<uintptr_t>(stack_bottom);
  // Add the context structure at the start of the stack.
  return boost::intrusive_ptr<Context>{new (storage) context_t{
      name, boost::context::preallocated{storage, size, sctx}, salloc,
      std::forward<Fn>(fn), std::forward<Arg>(arg)...}};
}

template <typename Fn, typename... Arg>
boost::context::fiber TaskContext<Fn, Arg...>::Run(boost::context::fiber&& c) {
  auto fn = std::move(fn_);
  auto arg = std::move(arg_);
  std::apply(std::move(fn), std::move(arg));
  return Terminate();
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
