#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cctype>
#include <tuple>
#include <glm/glm.hpp>

/** @file util.hpp
 *  @brief Misc helpers: splits, trimming, hashing, color parsing, 3D keys.
 */

namespace vxl {

inline std::vector<std::string> split_ws(const std::string& s) {
  std::istringstream iss(s);
  std::vector<std::string> out;
  for (std::string t; iss >> t; ) out.push_back(t);
  return out;
}

inline std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);});
  return s;
}

inline std::string trim(const std::string& s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

inline std::optional<glm::vec4> parse_rgba_hex(const std::string& s) {
  // Accept: #RRGGBB, #RRGGBBAA (hex)
  auto hex = s;
  if (!hex.empty() && hex[0] == '#') hex.erase(0,1);
  if (hex.size() != 6 && hex.size() != 8) return std::nullopt;
  auto byte = [&](int i)->float {
    return std::stoi(hex.substr(i,2), nullptr, 16) / 255.0f;
  };
  float r = byte(0), g = byte(2), b = byte(4), a = (hex.size()==8)? byte(6) : 1.0f;
  return glm::vec4(r,g,b,a);
}

// Key for integer 3D coord in unordered_map
struct IVec3 {
  int x, y, z;
  bool operator==(const IVec3& o) const noexcept { return x==o.x && y==o.y && z==o.z; }
};
struct IVec3Hasher {
  std::size_t operator()(const IVec3& k) const noexcept {
    // 3D hashing (mix)
    std::size_t h = 1469598103934665603ull;
    auto mix = [&](int v) {
      std::size_t x = static_cast<std::size_t>(static_cast<uint32_t>(v));
      h ^= x + 0x9e3779b97f4a7c15ull + (h<<6) + (h>>2);
    };
    mix(k.x); mix(k.y); mix(k.z);
    return h;
  }
};

} // namespace vxl
