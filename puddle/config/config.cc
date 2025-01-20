#include "puddle/config/config.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

// Returns whether the character identifies a special shell variable such as
// $*.
bool IsShellSpecialVar(char c) {
  switch (c) {
    case '*':
    case '#':
    case '$':
    case '@':
    case '!':
    case '?':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return true;
    default:
      return false;
  }
}

// Returns whether the char is an ASCII letter, number, or underscore.
bool IsAlphaNum(char c) {
  return (c == '_' || ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z'));
}

std::pair<std::string, size_t> GetShellName(const std::string& s) {
  if (s[0] == '{') {
    if (s.size() > 2 && IsShellSpecialVar(s[1]) && s[2] == '}') {
      return std::make_pair(s.substr(1, 1), 3);
    }
    // Scan to closing brace.
    for (size_t i = 0; i < s.size(); i++) {
      if (s[i] == '}') {
        if (i == 1) {
          // Bad syntax; eat "${}".
          return std::make_pair("", 2);
        }
        return std::make_pair(s.substr(1, i - 1), i + 1);
      }
    }
    // Bad syntax; eat "${".
    return std::make_pair("", 1);
  }

  if (IsShellSpecialVar(s[0])) {
    return std::make_pair(s.substr(0, 1), 1);
  }

  // Scan alphanumerics.
  size_t i;
  for (i = 0; i < s.size() && IsAlphaNum(s[i]); i++) {
  }
  return std::make_pair(s.substr(0, i), i);
}

}  // namespace

namespace puddle {
namespace config {

Exception::Exception(const std::string& m) : m_{m} {}

Exception::Exception(const std::string& m, int error_code) : m_{m} {
  m_ += ": " + std::string(strerror(error_code));
}

// From Golang os.Expand.
std::string Expand(const std::string s,
                   std::function<std::string(const std::string&)> mapping) {
  std::vector<char> buf;
  size_t i = 0;
  for (size_t j = 0; j < s.size(); j++) {
    if (s[j] == '$' && j + 1 < s.size()) {
      for (size_t k = i; k < j; k++) {
        buf.push_back(s[k]);
      }

      auto [name, w] = GetShellName(s.substr(j + 1));
      if (name == "" && w > 0) {
        // Encountered invalid syntax; eat the
        // characters.
      } else if (name == "") {
        // Valid syntax, but $ was not followed by a
        // name. Leave the dollar character untouched.
        buf.push_back(s[j]);
      } else {
        auto value = mapping(name);
        for (size_t k = 0; k < value.size(); k++) {
          buf.push_back(value[k]);
        }
      }
      j += w;
      i = j + 1;
    }
  }
  if (buf.size() == 0) {
    return s;
  }
  return std::string(buf.data(), buf.size()) + s.substr(i);
}

std::string ExpandEnv(const std::string s) {
  return Expand(s, [](const std::string& k) -> std::string {
    char* c = std::getenv(k.c_str());
    if (c) return std::string(c);
    return "";
  });
}

YAML::Node LoadYaml(const std::string& path, bool expand_env) {
  if (path == "") {
    return YAML::Node{};
  }

  std::string s = ReadFile(path);
  if (expand_env) {
    s = ExpandEnv(s);
  }

  try {
    return YAML::Load(s);
  } catch (const YAML::Exception& e) {
    throw Exception{"parse yaml: " + e.msg};
  }
}

}  // namespace config
}  // namespace puddle
