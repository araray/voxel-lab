#include "menu.hpp"
#include <fstream>
#include "util.hpp"

namespace vxl {

void MenuRegistry::clear() { items_.clear(); }
void MenuRegistry::add(const std::string& label, const std::string& command) { items_.push_back({label,command}); }
const std::vector<MenuItem>& MenuRegistry::items() const { return items_; }

bool MenuRegistry::load_from_file(const std::string& path) {
  std::ifstream f(path);
  if (!f) return false;
  items_.clear();
  std::string line;
  while (std::getline(f, line)) {
    line = trim(line);
    if (line.empty() || line[0]=='#') continue;
    auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    auto label = trim(line.substr(0,eq));
    auto cmd = trim(line.substr(eq+1));
    if (!label.empty() && !cmd.empty()) add(label, cmd);
  }
  return true;
}

} // namespace vxl
