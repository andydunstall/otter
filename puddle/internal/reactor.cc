#include "puddle/internal/reactor.h"

#include <cstring>

namespace puddle {
namespace internal {

// ReactorContext is the context to run the reactor.
class ReactorContext final : public Context {
 public:
  ReactorContext(const boost::context::preallocated& palloc,
                 boost::context::fixedsize_stack salloc, Reactor* reactor);

  // Allocates the reactor context and stack.
  static boost::intrusive_ptr<Context> Spawn(Reactor* reactor);

 private:
  boost::context::fiber Run(boost::context::fiber&& c);

  Reactor* reactor_;
};

ReactorContext::ReactorContext(const boost::context::preallocated& palloc,
                               boost::context::fixedsize_stack salloc,
                               Reactor* reactor)
    : Context{}, reactor_{reactor} {
  context_ = boost::context::fiber{
      std::allocator_arg, palloc, salloc,
      std::bind(&ReactorContext::Run, this, std::placeholders::_1)};
}

boost::context::fiber ReactorContext::Run(boost::context::fiber&& c) {
  reactor_->Dispatch();
  return boost::context::fiber{};
}

boost::intrusive_ptr<Context> ReactorContext::Spawn(Reactor* reactor) {
  // Allocate a stack for the context, then add the context structure at the
  // start of the stack.
  //
  // Copied from boost fibers make_worker_context_with_properties.

  boost::context::fixedsize_stack salloc{kStackSize};
  auto sctx = salloc.allocate();

  void* storage =
      reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(sctx.sp) -
                               static_cast<uintptr_t>(sizeof(ReactorContext))) &
                              ~static_cast<uintptr_t>(0xff));
  void* stack_bottom = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
  const std::size_t size = reinterpret_cast<uintptr_t>(storage) -
                           reinterpret_cast<uintptr_t>(stack_bottom);
  return boost::intrusive_ptr<Context>{new (storage) ReactorContext{
      boost::context::preallocated{storage, size, sctx}, salloc, reactor}};
}

BlockingRequest::BlockingRequest() : ctx_(Reactor::local()->active_) {}

int BlockingRequest::Wait() {
  // Suspend the current fiber, then the reactor will wake us up once the
  // result is ready.
  Reactor::local()->Suspend();
  return result_;
}

void BlockingRequest::Connect(int sockfd, struct sockaddr* addr,
                              socklen_t addrlen) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_connect(sqe, sockfd, addr, addrlen);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Accept(int sockfd, struct sockaddr* addr,
                             socklen_t* addrlen, int flags) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_accept(sqe, sockfd, addr, addrlen, flags);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Read(int fd, void* buf, unsigned nbytes, off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_read(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Write(int fd, const void* buf, unsigned nbytes,
                            off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_write(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::SetResult(int result) {
  result_ = result;
  Reactor::local()->Schedule(ctx_);
}

Reactor::Config Reactor::Config::Default() {
  Config config;
  config.ring_size = 1024;
  return config;
}

Reactor::Reactor(Config config) : logger_{"reactor"} {
  int res = io_uring_queue_init(config.ring_size, &ring_, 0);
  if (res != 0) {
    logger_.Fatal("failed to setup io_uring: {}", strerror(-res));
  }

  reactor_context_ = internal::ReactorContext::Spawn(this);
  scheduler_.AddReady(reactor_context_.get());

  // The main context is the currently active context.
  active_ = &main_context_;
}

Reactor::~Reactor() { io_uring_queue_exit(&ring_); }

void Reactor::Yield() {
  // The reactor context is always ready (unless we're in the reactor
  // context) so we'll always have another context to switch to.
  internal::Context* next = scheduler_.NextReady();
  assert(next != nullptr);

  // Add the current context to the ready queue.
  internal::Context* prev = active_;
  scheduler_.AddReady(prev);
  active_ = next;

  // Switch to the new context. As the underlying Boost context is "one shot",
  // we must update prev->context_ to the new state.
  //
  // We ignore the returned context as it's already in the ready queue.
  std::move(active_->context_).resume_with([prev](boost::context::fiber&& c) {
    prev->context_ = std::move(c);
    return boost::context::fiber{};
  });
}

void Reactor::Suspend() {
  // The reactor context is always ready (unless we're in the reactor
  // context) so we'll always have another context to switch to.
  internal::Context* next = scheduler_.NextReady();
  assert(next != nullptr);

  internal::Context* prev = active_;
  active_ = next;

  // Switch to the new context. As the underlying Boost context is "one shot",
  // we must update prev->context_ to the new state.
  //
  // We ignore the returned context as it's already been added to prev.
  std::move(active_->context_).resume_with([prev](boost::context::fiber&& c) {
    prev->context_ = std::move(c);
    return boost::context::fiber{};
  });
}

void Reactor::Schedule(Context* context) { scheduler_.AddReady(context); }

void Reactor::Dispatch() {
  // TODO(andydunstall)
}

void Reactor::Start(Config config) {
  // Set local reactor.
  local_ = new Reactor{config};
}

thread_local Reactor* Reactor::local_ = nullptr;

}  // namespace internal
}  // namespace puddle
