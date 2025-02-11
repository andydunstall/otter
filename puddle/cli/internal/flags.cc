#include "puddle/cli/internal/flags.h"

namespace puddle {
namespace cli {
namespace internal {

BaseFlag::BaseFlag(const std::string& name, const std::string& description)
    : name_{name}, description_{description} {}

}  // namespace internal
}  // namespace cli
}  // namespace puddle
