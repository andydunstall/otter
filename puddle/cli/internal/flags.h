#pragma once

#include <string>

#include "absl/strings/numbers.h"
#include "puddle/log/level.h"

namespace puddle {
namespace cli {
namespace internal {

class BaseFlag {
 public:
  BaseFlag(const std::string& name, const std::string& description);

  virtual ~BaseFlag() = default;

  std::string name() const { return name_; }

  std::string description() const { return description_; }

  virtual void Parse(const std::string& s) = 0;

 protected:
  std::string name_;

  std::string description_;
};

template <typename T>
class TypedFlag : public BaseFlag {
  static_assert(sizeof(T) == 0, "type not supported");
};

template <>
class TypedFlag<std::string> : public BaseFlag {
 public:
  TypedFlag(const std::string& name, std::string* value,
            const std::string& description)
      : BaseFlag{name, description}, value_{value} {}

  void Parse(const std::string& s) override { *value_ = s; }

 private:
  std::string* value_;
};

template <>
class TypedFlag<int> : public BaseFlag {
 public:
  TypedFlag(const std::string& name, int* value, const std::string& description)
      : BaseFlag{name, description}, value_{value} {}

  void Parse(const std::string& s) override {
    if (!absl::SimpleAtoi(s, value_)) {
      // ...
    }
  }

 private:
  int* value_;
};

template <>
class TypedFlag<log::Level> : public BaseFlag {
 public:
  TypedFlag(const std::string& name, log::Level* value,
            const std::string& description)
      : BaseFlag{name, description}, value_{value} {}

  void Parse(const std::string& s) override {
    *value_ = log::LevelFromString(s);
  }

 private:
  log::Level* value_;
};

}  // namespace internal
}  // namespace cli
}  // namespace puddle
