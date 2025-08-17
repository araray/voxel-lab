#include "selection.hpp"
#include <glm/gtx/quaternion.hpp>
#include <limits>
#include <cmath>

namespace vxl {

void Selection::clear() { set_.clear(); }
void Selection::add(const IVec3& p) { set_.insert(p); }
void Selection::toggle(const IVec3& p) {
  if (set_.count(p)) set_.erase(p); else set_.insert(p);
}
bool Selection::contains(const IVec3& p) const { return set_.count(p) != 0; }
std::vector<IVec3> Selection::items() const { return std::vector<IVec3>(set_.begin(), set_.end()); }

static bool ray_aabb(const glm::vec3& ro, const glm::vec3& rd,
                     const glm::vec3& minB, const glm::vec3& maxB, float& tOut) {
  // Slab method
  float tmin = -std::numeric_limits<float>::infinity();
  float tmax =  std::numeric_limits<float>::infinity();
  for (int i=0;i<3;i++) {
    float invD = 1.0f / rd[i];
    float t0 = (minB[i] - ro[i]) * invD;
    float t1 = (maxB[i] - ro[i]) * invD;
    if (invD < 0.0f) std::swap(t0,t1);
    tmin = std::max(tmin, t0);
    tmax = std::min(tmax, t1);
    if (tmax < tmin) return false;
  }
  tOut = tmin;
  return tOut >= 0.0f;
}

std::optional<IVec3> Selection::pick_cube(const Universe& U,
                                          const glm::vec3& ro,
                                          const glm::vec3& rd) {
  float bestT = std::numeric_limits<float>::infinity();
  std::optional<IVec3> best{};
  for (auto&& [p, c] : U.all()) {
    glm::vec3 minB{p.x - 0.5f, p.y - 0.5f, p.z - 0.5f};
    glm::vec3 maxB{p.x + 0.5f, p.y + 0.5f, p.z + 0.5f};
    float t;
    if (ray_aabb(ro, rd, minB, maxB, t)) {
      if (t < bestT) { bestT = t; best = p; }
    }
  }
  return best;
}

void Selection::move(Universe& U, const IVec3& d) {
  if (set_.empty()) return;
  // Copy, then move to avoid collisions
  std::vector<std::pair<IVec3, Cube>> temp;
  temp.reserve(set_.size());
  for (auto& p : set_) {
    auto c = U.get(p.x,p.y,p.z);
    if (c) temp.push_back({p, *c});
  }
  for (auto& [p,c] : temp) U.erase(p.x,p.y,p.z);
  std::unordered_set<IVec3, IVec3Hash> newset;
  for (auto& [p,c] : temp) {
    IVec3 np{p.x + d.x, p.y + d.y, p.z + d.z};
    U.place(np.x,np.y,np.z, c);
    newset.insert(np);
  }
  set_.swap(newset);
}

void Selection::rotate(Universe& U, char axis, float degrees) {
  if (set_.empty()) return;
  // Simple rotation: rotate each cube's local rotation; keep positions unchanged (framework base).
  float rad = glm::radians(degrees);
  glm::vec3 ax = (axis=='x') ? glm::vec3(1,0,0) : (axis=='y') ? glm::vec3(0,1,0) : glm::vec3(0,0,1);
  glm::quat rq = glm::angleAxis(rad, glm::normalize(ax));
  for (auto& p : set_) {
    auto c = U.get(p.x,p.y,p.z);
    if (!c) continue;
    c->rotation = rq * c->rotation;
    U.place(p.x,p.y,p.z, *c);
  }
}

} // namespace vxl
