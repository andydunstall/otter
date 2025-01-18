#pragma once

#include <yaml-cpp/yaml.h>

#include <string>

namespace puddle {
namespace config {

class Exception : public std::exception {
 public:
  Exception(const std::string& m);

  Exception(const std::string& m, int error_code);

  const char* what() const noexcept override { return m_.c_str(); }

 private:
  std::string m_;
};

YAML::Node LoadYaml(const std::string& path, bool expand_env);

}  // namespace config
}  // namespace puddle
