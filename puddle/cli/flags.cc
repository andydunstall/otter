#include "puddle/cli/flags.h"

#include <iostream>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"

namespace puddle {
namespace cli {

void Flags::Parse(int argc, char* argv[]) {
  std::vector<std::string> args;
  for (int i = 1; i < argc; i++) {
    args.emplace_back(argv[i]);
  }
  Parse(args);
}

void Flags::Parse(const std::vector<std::string>& args) {
  for (size_t i = 0; i < args.size(); i++) {
    const std::string& arg = args[i];

    if (arg == "-h" || arg == "--help") {
      help_ = true;
      continue;
    }

    if (!absl::StartsWith(arg, "--")) {
      // error
      continue;
    }
    size_t pos = arg.find('=');
    if (pos != std::string::npos) {
      const std::string name = arg.substr(2, pos - 2);
      const std::string value = arg.substr(pos + 1);

      // TODO if either name or value is empty, then fail

      if (flags_.find(name) == flags_.end()) {
        // not found
      }

      flags_[name]->Parse(value);
    } else {
      if (i == args.size() - 1) {
        // missing value
      }
      i++;

      const std::string name = arg.substr(2);
      const std::string value = args[i];

      if (flags_.find(name) == flags_.end()) {
        // not found
      }
      flags_[name]->Parse(value);
    }
  }
}

}  // namespace cli
}  // namespace puddle
