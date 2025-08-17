#pragma once
#include <SDL.h>

/** @file input.hpp
 *  @brief Lightweight input mapping utilities.
 */
namespace vxl {

struct InputState {
  bool lmb=false, rmb=false, mmb=false;
  int wheel=0; // scroll delta
  int mx=0,my=0, dmx=0, dmy=0; // mouse pos and delta
  bool alt=false, shift=false, ctrl=false;
  bool key_plus=false, key_minus=false;
  bool key_w=false, key_a=false, key_s=false, key_d=false, key_q=false, key_e=false;
  bool key_g=false, key_f2=false, key_f1=false, key_bracket_l=false, key_bracket_r=false, key_r=false;
};

inline void begin_frame(InputState& in) { in.dmx=0; in.dmy=0; in.wheel=0; }

inline void handle_event(InputState& in, const SDL_Event& e) {
  if (e.type == SDL_MOUSEMOTION) { in.dmx += e.motion.xrel; in.dmy += e.motion.yrel; in.mx = e.motion.x; in.my = e.motion.y; }
  else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
    bool down = (e.type == SDL_MOUSEBUTTONDOWN);
    if (e.button.button == SDL_BUTTON_LEFT) in.lmb = down;
    if (e.button.button == SDL_BUTTON_RIGHT) in.rmb = down;
    if (e.button.button == SDL_BUTTON_MIDDLE) in.mmb = down;
  } else if (e.type == SDL_MOUSEWHEEL) {
    in.wheel += e.wheel.y;
  } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
    bool down = (e.type == SDL_KEYDOWN);
    SDL_Keycode k = e.key.keysym.sym;
    if (k==SDLK_LALT || k==SDLK_RALT) in.alt = down;
    if (k==SDLK_LSHIFT || k==SDLK_RSHIFT) in.shift = down;
    if (k==SDLK_LCTRL || k==SDLK_RCTRL) in.ctrl = down;
    if (k==SDLK_EQUALS || k==SDLK_KP_PLUS) in.key_plus = down;
    if (k==SDLK_MINUS  || k==SDLK_KP_MINUS) in.key_minus = down;
    if (k==SDLK_w) in.key_w = down;
    if (k==SDLK_a) in.key_a = down;
    if (k==SDLK_s) in.key_s = down;
    if (k==SDLK_d) in.key_d = down;
    if (k==SDLK_q) in.key_q = down;
    if (k==SDLK_e) in.key_e = down;
    if (k==SDLK_g) in.key_g = down;
    if (k==SDLK_F2) in.key_f2 = down;
    if (k==SDLK_F1) in.key_f1 = down;
    if (k==SDLK_LEFTBRACKET) in.key_bracket_l = down;
    if (k==SDLK_RIGHTBRACKET) in.key_bracket_r = down;
    if (k==SDLK_r) in.key_r = down;
  }
}

} // namespace vxl
