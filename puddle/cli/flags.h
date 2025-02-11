#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "puddle/cli/internal/flags.h"

namespace puddle {
namespace cli {

class Flags {
 public:
  template <typename T>
  void Add(const std::string& name, T* value, const std::string& description);

  bool help() const { return help_; }

  void Parse(int argc, char* argv[]);

  void Parse(const std::vector<std::string>& args);

 private:
  std::map<std::string, std::unique_ptr<internal::BaseFlag>> flags_;

  bool help_ = false;
};

template <typename T>
void Flags::Add(const std::string& name, T* value,
                const std::string& description) {
  flags_[name] =
      std::make_unique<internal::TypedFlag<T>>(name, value, description);
}

}  // namespace cli
}  // namespace puddle
