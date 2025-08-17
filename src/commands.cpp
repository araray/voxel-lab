// src/commands.cpp
#include "commands.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <glm/gtc/constants.hpp>

namespace vxl {

// ----- CommandRegistry impl -----

void CommandRegistry::register_cmd(const std::string& name, const std::string& help, CommandFn fn) {
  map_[to_lower(name)] = Entry{help, std::move(fn)};
}

bool CommandRegistry::run_line(const std::string& line, CommandContext& ctx) {
  auto tokens = split_ws(line);
  if (tokens.empty()) return false;
  auto cmd = to_lower(tokens[0]);
  tokens.erase(tokens.begin());
  auto it = map_.find(cmd);
  if (it == map_.end()) { ctx.print("Unknown command: " + cmd); return false; }
  try {
    it->second.fn(tokens, ctx);
    return true;
  } catch (std::exception& e) {
    ctx.print(std::string("Error: ") + e.what());
    return false;
  }
}

std::vector<std::string> CommandRegistry::completions(const std::string& prefix) const {
  std::vector<std::string> out;
  for (auto& [k,v] : map_) if (k.rfind(prefix,0) == 0) out.push_back(k);
  return out;
}

std::string CommandRegistry::help_all() const {
  std::ostringstream os;
  for (auto& [k,v] : map_) os << k << " - " << v.help << "\n";
  return os.str();
}

// ----- helpers -----

static bool parse_int3(const std::vector<std::string>& a, int i, int& x,int& y,int& z) {
  if (i+2 >= (int)a.size()) return false;
  x = std::stoi(a[i]); y = std::stoi(a[i+1]); z = std::stoi(a[i+2]);
  return true;
}

// ----- builtins -----

void register_builtin_commands(CommandRegistry& R) {
  // place
  R.register_cmd("place", "place x y z [color=#RRGGBBAA] [gradient=#..,#..] [dir=x|y|z]",
    [](const auto& t, CommandContext& ctx){
      int x,y,z; if (!parse_int3(t,0,x,y,z)) { ctx.print("Usage: place x y z [color=#..] [gradient=#..,#..] [dir=x|y|z]"); return; }
      Cube c;
      for (size_t i=3;i<t.size();++i) {
        auto kv = t[i];
        auto eq = kv.find('=');
        if (eq == std::string::npos) continue;
        auto k = to_lower(kv.substr(0,eq));
        auto v = kv.substr(eq+1);
        if (k == "color") {
          auto col = parse_rgba_hex(v); if (col) { c.mat.kind = Material::Kind::Solid; c.mat.colorA = *col; }
        } else if (k == "gradient") {
          auto pos = v.find(',');
          if (pos!=std::string::npos) {
            auto c1 = parse_rgba_hex(v.substr(0,pos));
            auto c2 = parse_rgba_hex(v.substr(pos+1));
            if (c1 && c2) { c.mat.kind = Material::Kind::Gradient; c.mat.colorA=*c1; c.mat.colorB=*c2; }
          }
        } else if (k == "dir") {
          auto d = to_lower(v);
          if (d=="x") c.mat.gradDir={1,0,0};
          else if (d=="y") c.mat.gradDir={0,1,0};
          else c.mat.gradDir={0,0,1};
        }
      }
      ctx.U.place(x,y,z, c);
      ctx.request_redraw();
    }
  );

  // erase
  R.register_cmd("erase", "erase x y z | erase selection",
    [](const auto& t, CommandContext& ctx){
      if (t.size()==1 && to_lower(t[0])=="selection") {
        auto items = ctx.Sel.items();
        for (auto& p : items) ctx.U.erase(p.x,p.y,p.z);
        ctx.Sel.clear();
        ctx.request_redraw();
        return;
      }
      int x,y,z; if (!parse_int3(t,0,x,y,z)) { ctx.print("Usage: erase x y z | erase selection"); return; }
      ctx.U.erase(x,y,z);
      ctx.request_redraw();
    }
  );

  // select
  R.register_cmd("select", "select x y z | select box x1 y1 z1 x2 y2 z2",
    [](const auto& t, CommandContext& ctx){
      if (!t.empty() && to_lower(t[0])=="box") {
        if (t.size()<7) { ctx.print("Usage: select box x1 y1 z1 x2 y2 z2"); return; }
        int x1=std::stoi(t[1]),y1=std::stoi(t[2]),z1=std::stoi(t[3]);
        int x2=std::stoi(t[4]),y2=std::stoi(t[5]),z2=std::stoi(t[6]);
        ctx.Sel.clear();
        for (int x=std::min(x1,x2); x<=std::max(x1,x2); ++x)
        for (int y=std::min(y1,y2); y<=std::max(y1,y2); ++y)
        for (int z=std::min(z1,z2); z<=std::max(z1,z2); ++z) {
          if (ctx.U.get(x,y,z)) ctx.Sel.add({x,y,z});
        }
        ctx.request_redraw();
        return;
      }
      int x,y,z; if (!parse_int3(t,0,x,y,z)) { ctx.print("Usage: select x y z | select box ..."); return; }
      ctx.Sel.clear(); ctx.Sel.add({x,y,z});
      ctx.request_redraw();
    }
  );

  // move
  R.register_cmd("move", "move dx dy dz (integers)",
    [](const auto& t, CommandContext& ctx){
      if (t.size()<3) { ctx.print("Usage: move dx dy dz"); return; }
      IVec3 d{std::stoi(t[0]), std::stoi(t[1]), std::stoi(t[2])};
      ctx.Sel.move(ctx.U, d);
      ctx.request_redraw();
    }
  );

  // rotate
  R.register_cmd("rotate", "rotate [x|y|z] degrees",
    [](const auto& t, CommandContext& ctx){
      if (t.size()<2) { ctx.print("Usage: rotate y 15"); return; }
      char ax = std::tolower(static_cast<unsigned char>(t[0][0]));
      float deg = std::stof(t[1]);
      ctx.Sel.rotate(ctx.U, ax, deg);
      ctx.request_redraw();
    }
  );

  // fill
  R.register_cmd("fill", "fill solid #RRGGBBAA | fill gradient c1 c2 [dir=x|y|z]",
    [](const auto& t, CommandContext& ctx){
      if (t.empty()) { ctx.print("Usage: fill solid ... | fill gradient ..."); return; }
      auto items = ctx.Sel.items();
      if (items.empty()) { ctx.print("Nothing selected."); return; }

      if (to_lower(t[0])=="solid") {
        if (t.size()<2) { ctx.print("Usage: fill solid #RRGGBBAA"); return; }
        auto col = parse_rgba_hex(t[1]); if (!col) { ctx.print("Bad color"); return; }
        for (auto& p: items) {
          auto c=ctx.U.get(p.x,p.y,p.z); if(c){ c->mat.kind=Material::Kind::Solid; c->mat.colorA=*col; ctx.U.place(p.x,p.y,p.z,*c); }
        }
      } else if (to_lower(t[0])=="gradient") {
        if (t.size()<3) { ctx.print("Usage: fill gradient c1 c2 [dir=x|y|z]"); return; }
        auto c1 = parse_rgba_hex(t[1]), c2 = parse_rgba_hex(t[2]); if(!c1||!c2){ ctx.print("Bad colors"); return; }
        glm::vec3 dir{0,1,0};
        if (t.size()>=4) {
          auto kv = t[3]; auto eq = kv.find('='); if (eq!=std::string::npos && to_lower(kv.substr(0,eq))=="dir") {
            auto v=to_lower(kv.substr(eq+1)); if(v=="x") dir={1,0,0}; else if(v=="y") dir={0,1,0}; else dir={0,0,1};
          }
        }
        for (auto& p: items) {
          auto c=ctx.U.get(p.x,p.y,p.z);
          if(c){ c->mat.kind=Material::Kind::Gradient; c->mat.colorA=*c1; c->mat.colorB=*c2; c->mat.gradDir=dir; ctx.U.place(p.x,p.y,p.z,*c); }
        }
      }
      ctx.request_redraw();
    }
  );

  // grid (UI hint)
  R.register_cmd("grid", "grid on|off|toggle",
    [](const auto& t, CommandContext& ctx){
      if (t.empty()) { ctx.print("Usage: grid on|off|toggle"); return; }
      ctx.print(std::string("[ui] grid ") + t[0]);
      ctx.request_redraw();
    }
  );

  // wireframe (UI hint)
  R.register_cmd("wireframe", "wireframe on|off|toggle",
    [](const auto& t, CommandContext& ctx){
      if (t.empty()) { ctx.print("Usage: wireframe on|off|toggle"); return; }
      ctx.print(std::string("[ui] wireframe ") + t[0]);
      ctx.request_redraw();
    }
  );

  // edgepix
  R.register_cmd("edgepix", "edgepix N -- set pixel size for cube edge",
    [](const auto& t, CommandContext& ctx){
      if (t.empty()) { ctx.print("Usage: edgepix N"); return; }
      int px = std::stoi(t[0]);
      ctx.U.set_base_edge_pixels(px);
      ctx.recompute_camera_edgepix();
      ctx.request_redraw();
    }
  );

  // group
  R.register_cmd("group", "group create NAME | select NAME | move NAME dx dy dz | erase NAME",
    [](const auto& t, CommandContext& ctx){
      if (t.size()<2) { ctx.print("Usage: group ..."); return; }
      auto sub = to_lower(t[0]); auto name = t[1];
      if (sub=="create") {
        ctx.U.group_create(name, ctx.Sel.items());
        ctx.print("Group created: " + name);
      } else if (sub=="select") {
        ctx.Sel.clear();
        for (auto& p : ctx.U.group_members(name)) ctx.Sel.add(p);
        ctx.request_redraw();
      } else if (sub=="move") {
        if (t.size()<5) { ctx.print("Usage: group move NAME dx dy dz"); return; }
        IVec3 d{std::stoi(t[2]), std::stoi(t[3]), std::stoi(t[4])};
        if (!ctx.U.group_move(name, d)) ctx.print("No such group");
        ctx.request_redraw();
      } else if (sub=="erase") {
        ctx.U.group_erase(name);
        ctx.print("Group erased: " + name);
      }
    }
  );

  // print
  R.register_cmd("print", "print arbitrary text (debug)",
    [](const auto& t, CommandContext& ctx){
      std::ostringstream os; for (auto& s: t) os << s << " ";
      ctx.print(os.str());
    }
  );

  // help (single, correct implementation)
  R.register_cmd("help", "List all commands",
    [&R](const auto&, CommandContext& ctx){
      ctx.print(R.help_all());
    }
  );
}

} // namespace vxl
