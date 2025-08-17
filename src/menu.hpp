#pragma once
#include <string>
#include <vector>

/** @file menu.hpp
 *  @brief Simple programmable context menu spec (loaded from resources/menus.txt).
 */

namespace vxl {

struct MenuItem {
  std::string label;
  std::string command;
};

class MenuRegistry {
public:
  void clear();
  void add(const std::string& label, const std::string& command);
  const std::vector<MenuItem>& items() const;
  bool load_from_file(const std::string& path);

private:
  std::vector<MenuItem> items_;
};

} // namespace vxl
