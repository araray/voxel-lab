#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ImGui
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#ifndef DEFAULT_RES_DIR
#define DEFAULT_RES_DIR "."
#endif

namespace vxl {

App::App() {
  init_sdl();
  Rend_.init_gl();
  init_imgui();

  // Load menus
  Menus_.load_from_file(std::string(DEFAULT_RES_DIR) + "/menus.txt");

  // Built-ins
  register_builtin_commands(Cmds_);

  // Seed: a few cubes
  for (int z=0; z<3; ++z) U_.place(z,0,0);
}

App::~App() { shutdown(); }

void App::init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
    throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  window_ = SDL_CreateWindow("Voxel Lab (SDL2/OpenGL3)",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window_) throw std::runtime_error("SDL_CreateWindow failed");

  glctx_ = SDL_GL_CreateContext(window_);
  if (!glctx_) throw std::runtime_error("SDL_GL_CreateContext failed");
  SDL_GL_MakeCurrent(window_, glctx_);
  SDL_GL_SetSwapInterval(1);

  int w,h; SDL_GetWindowSize(window_, &w, &h);
  Cam_.set_viewport(w,h);
  Cam_.set_distance_for_pixel_edge(U_.base_edge_pixels());
  Rend_.resize(w,h);
}

void App::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window_, glctx_);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}

void App::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  if (glctx_) SDL_GL_DeleteContext(glctx_);
  if (window_) SDL_DestroyWindow(window_);
  SDL_Quit();
}

void App::print(const std::string& s) { console_.push_back(s); if (console_.size()>512) console_.pop_front(); }
void App::request_redraw() { /* no-op; draw every frame */ }

void App::recompute_camera_edgepix() {
  Cam_.set_distance_for_pixel_edge(U_.base_edge_pixels());
}

void App::handle_shortcuts() {
  if (In_.key_plus) Cam_.zoom(+0.02f);
  if (In_.key_minus) Cam_.zoom(-0.02f);
  if (In_.key_g) showGrid_ = !showGrid_;
  if (In_.key_f2) showWireframe_ = !showWireframe_;
  if (In_.key_f1) showHelp_ = !showHelp_;

  float k = 0.05f * Cam_.distance();
  if (In_.key_w) Cam_.move_local(+k,0,0);
  if (In_.key_s) Cam_.move_local(-k,0,0);
  if (In_.key_a) Cam_.move_local(0,-k,0);
  if (In_.key_d) Cam_.move_local(0,+k,0);
  if (In_.key_q) Cam_.move_local(0,0,-k);
  if (In_.key_e) Cam_.move_local(0,0,+k);

  // Rotate selection with bracket keys
  if (In_.key_bracket_l) Sel_.rotate(U_, 'y', -1.0f);
  if (In_.key_bracket_r) Sel_.rotate(U_, 'y', +1.0f);
}

std::optional<IVec3> App::pick_under_cursor() const {
  // Compute world ray from mouse
  int w,h; SDL_GetWindowSize(window_, &w, &h);
  float x = (2.0f * In_.mx) / float(w) - 1.0f;
  float y = 1.0f - (2.0f * In_.my) / float(h);
  glm::mat4 P = Cam_.proj();
  glm::mat4 V = Cam_.view();
  glm::mat4 invVP = glm::inverse(P * V);
  glm::vec4 pNear = invVP * glm::vec4(x,y,-1,1); pNear /= pNear.w;
  glm::vec4 pFar  = invVP * glm::vec4(x,y, 1,1); pFar  /= pFar.w;
  glm::vec3 ro = glm::vec3(pNear);
  glm::vec3 rd = glm::normalize(glm::vec3(pFar - pNear));
  return Selection::pick_cube(U_, ro, rd);
}

void App::handle_interaction() {
  // Orbit / pan / zoom
  if (In_.rmb && !ImGui::GetIO().WantCaptureMouse) {
    if (In_.shift) Cam_.pan(-0.005f * In_.dmx * Cam_.distance(), +0.005f * In_.dmy * Cam_.distance());
    else Cam_.orbit(-0.005f * In_.dmx, -0.005f * In_.dmy);
  }
  if (In_.mmb && !ImGui::GetIO().WantCaptureMouse) {
    Cam_.pan(-0.005f * In_.dmx * Cam_.distance(), +0.005f * In_.dmy * Cam_.distance());
  }
  if (In_.wheel != 0) Cam_.zoom(0.1f * In_.wheel);

  // Select
  if (In_.lmb && !ImGui::GetIO().WantCaptureMouse && (std::abs(In_.dmx)+std::abs(In_.dmy))==0) {
    if (auto hit = pick_under_cursor()) {
      if (In_.ctrl) Sel_.toggle(*hit); else { Sel_.clear(); Sel_.add(*hit); }
    }
  }

  // Drag move (simple: along camera plane)
  static bool dragging = false;
  static glm::vec3 dragStartEye;
  if (In_.lmb && !ImGui::GetIO().WantCaptureMouse && !Sel_.items().empty()) {
    dragging = true;
    dragStartEye = Cam_.eye();
    // Move selection by mouse delta projected into camera right/up
    glm::mat4 V = Cam_.view();
    glm::vec3 right{V[0][0], V[1][0], V[2][0]};
    glm::vec3 up{V[0][1], V[1][1], V[2][1]};
    float scale = 0.01f * Cam_.distance();
    IVec3 d{ (int)std::round(scale * In_.dmx * right.x),
             (int)std::round(scale * -In_.dmy * up.y),
             0 };
    if (d.x || d.y || d.z) Sel_.move(U_, d);
  } else {
    dragging = false;
  }

  // Rotate with R + mouse X
  if (In_.key_r && !ImGui::GetIO().WantCaptureKeyboard) {
    float deg = 0.2f * In_.dmx;
    Sel_.rotate(U_, 'y', deg);
  }
}

void App::ui_console() {
  ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 160), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 20, 150), ImGuiCond_Always);
  ImGui::Begin("Console", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

  // Output area
  ImGui::BeginChild("scroll", ImVec2(0, -30), true);
  for (auto& line : console_) ImGui::TextUnformatted(line.c_str());
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();

  // Input line
  ImGui::PushItemWidth(-1);
  if (ImGui::InputText("##input", &inputLine_, ImGuiInputTextFlags_EnterReturnsTrue)) {
    // Execute
    CommandContext ctx{
      U_, Sel_,
      [this](const std::string& s){ this->print(s); },
      [this](){ this->request_redraw(); },
      [this](){ this->recompute_camera_edgepix(); }
    };
    bool ok = Cmds_.run_line(inputLine_, ctx);
    if (ok) { history_.push_back(inputLine_); historyPos_ = -1; }
    inputLine_.clear();
  }
  // History with Up/Down
  if (ImGui::IsItemFocused()) {
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !history_.empty()) {
      if (historyPos_ < 0) historyPos_ = (int)history_.size()-1;
      else if (historyPos_ > 0) --historyPos_;
      inputLine_ = history_[historyPos_];
    } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && historyPos_>=0) {
      if (++historyPos_ >= (int)history_.size()) { historyPos_ = -1; inputLine_.clear(); }
      else inputLine_ = history_[historyPos_];
    }
  }
  ImGui::PopItemWidth();
  ImGui::End();
}

void App::ui_context_menu() {
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::GetIO().WantCaptureMouse) {
    ImGui::OpenPopup("ctx");
  }
  if (ImGui::BeginPopup("ctx")) {
    for (auto& it : Menus_.items()) {
      if (ImGui::MenuItem(it.label.c_str())) {
        CommandContext ctx{U_, Sel_,
          [this](const std::string& s){ this->print(s); },
          [this](){ this->request_redraw(); },
          [this](){ this->recompute_camera_edgepix(); }
        };
        Cmds_.run_line(it.command, ctx);
      }
    }
    ImGui::Separator();
    if (ImGui::MenuItem(showGrid_ ? "Grid: ON" : "Grid: OFF")) showGrid_ = !showGrid_;
    if (ImGui::MenuItem(showWireframe_ ? "Wireframe: ON" : "Wireframe: OFF")) showWireframe_ = !showWireframe_;
    ImGui::EndPopup();
  }
}

void App::ui_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (showHelp_) {
    ImGui::Begin("Help", &showHelp_);
    ImGui::Text("WASD/E/Q move, RMB orbit, MMB pan, Wheel zoom");
    ImGui::Text("LMB select, drag to move; R + mouse rotate; [ ] rotate Y");
    ImGui::Text("G grid, F2 wireframe, F1 help");
    ImGui::End();
  }

  ui_console();
  ui_context_menu();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int App::run() {
  while (running_) {
    begin_frame(In_);
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      ImGui_ImplSDL2_ProcessEvent(&e);
      if (e.type == SDL_QUIT) running_ = false;
      else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        int w=e.window.data1, h=e.window.data2;
        Cam_.set_viewport(w,h);
        Rend_.resize(w,h);
        Cam_.set_distance_for_pixel_edge(U_.base_edge_pixels());
      } else {
        handle_event(In_, e);
      }
    }

    handle_shortcuts();
    handle_interaction();

    // Render scene
    glm::mat4 V = Cam_.view();
    glm::mat4 P = Cam_.proj();
    Rend_.set_wireframe(showWireframe_);
    Rend_.render(U_, Sel_, V, P, showGrid_);

    // Draw UI on top
    ui_frame();

    SDL_GL_SwapWindow(window_);
  }
  return 0;
}

} // namespace vxl
