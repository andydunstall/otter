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

// Expand is the same as ExpandEnv, though accepts a custom mapping instead
// of using environment variables.
std::string Expand(const std::string s,
                   std::function<std::string(const std::string&)> mapping);

// ExpandEnv replaces reference to ${VAR} or $VAR with the corresponding
// environment variable. The replacement is case-sensitive.
//
// References to undefined variables will be replaced with an empty string.
std::string ExpandEnv(const std::string s);

YAML::Node LoadYaml(const std::string& path, bool expand_env);

}  // namespace config
}  // namespace puddle
