#include "puddle/config/config.h"

#include <stdlib.h>

#include <cstring>

namespace {

std::string ReadFile(const std::string& path) {
  FILE* file = fopen(path.c_str(), "r");
  if (!file) {
    throw puddle::config::Exception{"open config: " + path, errno};
  }

  std::string s;

  uint8_t buf[1024];
  size_t read_n;
  while ((read_n = fread(buf, 1, 1024, file)) > 0) {
    std::string read((const char*)buf, read_n);
    s += read;
  }

  if (ferror(file)) {
    throw puddle::config::Exception{"read config: " + path, errno};
  }

  fclose(file);

  return s;
}

}  // namespace

namespace puddle {
namespace config {

Exception::Exception(const std::string& m) : m_{m} {}

Exception::Exception(const std::string& m, int error_code) : m_{m} {
  m_ += ": " + std::string(strerror(error_code));
}

YAML::Node LoadYaml(const std::string& path, bool expand_env) {
  if (path == "") {
    return YAML::Node{};
  }

  std::string s = ReadFile(path);
  if (expand_env) {
    // TODO(andydunstall): Expand.
  }

  try {
    return YAML::Load(s);
  } catch (const YAML::Exception& e) {
    throw Exception{"parse yaml: " + e.msg};
  }
}

}  // namespace config
}  // namespace puddle
