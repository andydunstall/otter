#pragma once

#include <string>

namespace puddle {
namespace net {

class Exception : public std::exception {
 public:
  Exception(const std::string& m);

  Exception(const std::string& m, int error_code);

  const char* what() const noexcept override { return m_.c_str(); }

 private:
  std::string m_;
};

}  // namespace net
}  // namespace puddle
