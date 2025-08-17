#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include "universe.hpp"
#include "selection.hpp"
#include "util.hpp"

/** @file commands.hpp
 *  @brief Command registry & parser for the bottom prompt and menus.
 */

namespace vxl {

struct CommandContext {
  Universe& U;
  Selection& Sel;
  std::function<void(const std::string&)> print;  ///< output to console
  std::function<void()> request_redraw;           ///< call to redraw
  std::function<void()> recompute_camera_edgepix; ///< recompute camera distance from universe base edge pixels
};

using CommandFn = std::function<void(const std::vector<std::string>&, CommandContext&)>;

class CommandRegistry {
public:
  void register_cmd(const std::string& name, const std::string& help, CommandFn fn);
  bool run_line(const std::string& line, CommandContext& ctx);
  std::vector<std::string> completions(const std::string& prefix) const;
  std::string help_all() const;

private:
  struct Entry { std::string help; CommandFn fn; };
  std::unordered_map<std::string, Entry> map_;
};

// Register built-in commands
void register_builtin_commands(CommandRegistry& R);

} // namespace vxl
