#include "puddle/reactor/reactor.h"

#include <thread>

namespace puddle {
namespace reactor {

namespace internal {

class ReactorContext final : public Context {
 public:
  ReactorContext(const boost::context::preallocated& palloc,
                 boost::context::fixedsize_stack salloc, Reactor* reactor);

  // Allocates the reactor context and stack.
  static boost::intrusive_ptr<Context> Allocate(Reactor* reactor);

 private:
  boost::context::fiber Run(boost::context::fiber&& c);

  Reactor* reactor_;
};

}  // namespace internal

BlockingRequest::BlockingRequest() : ctx_(local()->active_) {}

int BlockingRequest::Wait() {
  // Suspend the current fiber, then the reactor will wake us up once the
  // result is ready.
  local()->Suspend();
  return result_;
}

void BlockingRequest::PrepConnect(int sockfd, struct sockaddr* addr,
                                  socklen_t addrlen) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&local()->ring_);
  io_uring_prep_connect(sqe, sockfd, addr, addrlen);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::PrepAccept(int sockfd, struct sockaddr* addr,
                                 socklen_t* addrlen, int flags) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&local()->ring_);
  io_uring_prep_accept(sqe, sockfd, addr, addrlen, flags);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::PrepRead(int fd, void* buf, unsigned nbytes,
                               off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&local()->ring_);
  io_uring_prep_read(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::PrepWrite(int fd, const void* buf, unsigned nbytes,
                                off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&local()->ring_);
  io_uring_prep_write(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::SetResult(int result) {
  result_ = result;
  local()->scheduler_.AddReady(ctx_);
}

Reactor::Reactor(Config config) : main_context_{"main"}, logger_{"reactor"} {
  int res = io_uring_queue_init(config.ring_size, &ring_, 0);
  if (res != 0) {
    logger_.Fatal("failed to setup io_uring: {}", strerror(-res));
  }

  reactor_context_ = internal::ReactorContext::Allocate(this);
  scheduler_.AddReady(reactor_context_.get());

  active_ = &main_context_;
}

Reactor::~Reactor() { io_uring_queue_exit(&ring_); }

void Reactor::Yield() {
  // The reactor context is always ready so we're guaranteed to have another
  // context to switch to.
  internal::Context* next = scheduler_.Next();

  logger_.Debug("yield; from = {}, to = {}", active_->name(), next->name());

  internal::Context* prev = active_;
  scheduler_.AddReady(prev);
  active_ = next;

  // TODO next.SwitchTo();
  std::move(active_->context_).resume_with([prev](boost::context::fiber&& c) {
    prev->context_ = std::move(c);
    return boost::context::fiber{};
  });
}

void Reactor::Suspend() {
  // The reactor context is always ready so we're guaranteed to have another
  // context to switch to.
  internal::Context* next = scheduler_.Next();

  logger_.Debug("suspend; from = {}, to = {}", active_->name(), next->name());

  internal::Context* prev = active_;
  active_ = next;

  std::move(active_->context_).resume_with([prev](boost::context::fiber&& c) {
    prev->context_ = std::move(c);
    return boost::context::fiber{};
  });
}

boost::context::fiber Reactor::Terminate() {
  internal::Context* next = scheduler_.Terminate(active_);

  logger_.Debug("terminate; to = {}", next->name());

  internal::Context* prev = active_;
  active_ = next;

  return std::move(active_->context_)
      .resume_with([prev](boost::context::fiber&& c) {
        prev->context_ = std::move(c);
        return boost::context::fiber{};
      });
}

void Reactor::Dispatch() {
  while (true) {
    io_uring_submit_and_get_events(&ring_);
    DispatchEvents();

    scheduler_.ReleaseTerminated();

    if (scheduler_.has_ready()) {
      Yield();
    } else {
      logger_.Debug("block");
      __kernel_timespec* ts_arg = nullptr;
      struct io_uring_cqe* cqe_ptr = nullptr;
      io_uring_wait_cqes(&ring_, &cqe_ptr, 1, ts_arg, NULL);
      DispatchEvents();
    }
  }
}

void Reactor::Setup(Config config) {
  // Set local reactor.
  local_ = new Reactor{config};
}

void Reactor::DispatchEvents() {
  uint32_t cqe_count = 0;
  unsigned ring_head;
  struct io_uring_cqe* cqe;
  io_uring_for_each_cqe(&ring_, ring_head, cqe) {
    cqe_count++;

    BlockingRequest* req = (BlockingRequest*)cqe->user_data;
    req->SetResult(cqe->res);
  }
  if (cqe_count) {
    io_uring_cq_advance(&ring_, cqe_count);
  }
}

Reactor* local() { return Reactor::local(); }

thread_local Reactor* Reactor::local_ = nullptr;

namespace internal {

ReactorContext::ReactorContext(const boost::context::preallocated& palloc,
                               boost::context::fixedsize_stack salloc,
                               Reactor* reactor)
    : Context{"reactor"}, reactor_{reactor} {
  context_ = boost::context::fiber{
      std::allocator_arg, palloc, salloc,
      std::bind(&ReactorContext::Run, this, std::placeholders::_1)};
}

boost::context::fiber ReactorContext::Run(boost::context::fiber&& c) {
  reactor_->Dispatch();
  return boost::context::fiber{};
}

boost::intrusive_ptr<Context> ReactorContext::Allocate(Reactor* reactor) {
  // Copied from boost fibers make_worker_context_with_properties.

  boost::context::fixedsize_stack salloc{kStackSize};
  auto sctx = salloc.allocate();

  // Reserve space for control structure.
  void* storage =
      reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(sctx.sp) -
                               static_cast<uintptr_t>(sizeof(ReactorContext))) &
                              ~static_cast<uintptr_t>(0xff));
  void* stack_bottom = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
  const std::size_t size = reinterpret_cast<uintptr_t>(storage) -
                           reinterpret_cast<uintptr_t>(stack_bottom);
  // Placement new of context on top of fiber's stack.
  return boost::intrusive_ptr<Context>{new (storage) ReactorContext{
      boost::context::preallocated{storage, size, sctx}, salloc, reactor}};
}

}  // namespace internal

}  // namespace reactor
}  // namespace puddle
