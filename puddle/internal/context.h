#pragma once

namespace puddle {
namespace internal {

// Context represents the a tasks execution state.
class Context {
 public:
  friend void intrusive_ptr_add_ref(Context* c) noexcept;
  friend void intrusive_ptr_release(Context* c) noexcept;
};

inline void intrusive_ptr_add_ref(Context* c) noexcept {
  // TODO(andydunstall)
}

inline void intrusive_ptr_release(Context* c) noexcept {
  // TODO(andydunstall)
}

}  // namespace internal
}  // namespace puddle
