#pragma once

#include <functional>
#include <string>

#include "puddle/cli/flags.h"

namespace puddle {
namespace cli {

class Command {
 public:
  Command(const std::string& name, const std::string& description);

  Flags& flags();

  int Run(std::function<void()> f);
};

}  // namespace cli
}  // namespace puddle
