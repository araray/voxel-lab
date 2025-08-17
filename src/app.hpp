#pragma once
#include <string>
#include <deque>
#include <SDL.h>
#include "universe.hpp"
#include "selection.hpp"
#include "renderer.hpp"
#include "commands.hpp"
#include "menu.hpp"
#include "camera.hpp"
#include "input.hpp"

/** @file app.hpp
 *  @brief SDL + GL + ImGui integration; main loop, UI, interactions.
 */

namespace vxl {

class App {
public:
  App();
  ~App();
  int run();

private:
  // SDL / GL / ImGui
  SDL_Window* window_ = nullptr;
  SDL_GLContext glctx_{};
  bool running_ = true;

  // State
  Universe U_{64};
  Selection Sel_;
  Camera Cam_;
  Renderer Rend_;
  MenuRegistry Menus_;
  CommandRegistry Cmds_;
  InputState In_;
  bool showGrid_ = true;
  bool showWireframe_ = false;
  bool showHelp_ = true;

  // Prompt
  std::string inputLine_;
  std::deque<std::string> console_;
  std::vector<std::string> history_;
  int historyPos_ = -1;

  // Impl
  void init_sdl();
  void init_imgui();
  void shutdown();

  void ui_frame();
  void ui_console();
  void ui_context_menu();
  void handle_interaction();
  void handle_shortcuts();

  void print(const std::string& s);
  void request_redraw();
  void recompute_camera_edgepix();

  // Picking helper
  std::optional<IVec3> pick_under_cursor() const;
};

} // namespace vxl
