#include "puddle/net/exception.h"

#include <cstring>

namespace puddle {
namespace net {

Exception::Exception(const std::string& m) : m_{m} {}

Exception::Exception(const std::string& m, int error_code) : m_{m} {
  m_ += ": ";
  m_ += strerror(error_code);
}

}  // namespace net
}  // namespace puddle
