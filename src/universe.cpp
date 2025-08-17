#include "universe.hpp"
#include <cassert>

namespace vxl {

Universe::Universe(int baseEdgePixels) : baseEdgePixels_(std::max(1, baseEdgePixels)) {}

void Universe::place(int x,int y,int z, const Cube& c) {
  cubes_[IVec3{x,y,z}] = c;
}

bool Universe::erase(int x,int y,int z) {
  return cubes_.erase(IVec3{x,y,z}) > 0;
}

std::optional<Cube> Universe::get(int x,int y,int z) const {
  auto it = cubes_.find(IVec3{x,y,z});
  if (it == cubes_.end()) return std::nullopt;
  return it->second;
}

void Universe::group_create(const std::string& name, const std::vector<IVec3>& members) {
  groups_[name] = members;
}

bool Universe::group_exists(const std::string& name) const {
  return groups_.find(name) != groups_.end();
}

std::vector<IVec3> Universe::group_members(const std::string& name) const {
  auto it = groups_.find(name);
  return (it == groups_.end()) ? std::vector<IVec3>{} : it->second;
}

bool Universe::group_erase(const std::string& name) {
  return groups_.erase(name) > 0;
}

bool Universe::group_move(const std::string& name, const IVec3& d) {
  auto it = groups_.find(name);
  if (it == groups_.end()) return false;
  std::vector<IVec3> newMembers;
  newMembers.reserve(it->second.size());
  // Move actual cubes if they exist
  std::unordered_map<IVec3, Cube, IVec3Hasher> moved;
  for (auto& p : it->second) {
    auto c = get(p.x,p.y,p.z);
    if (!c) continue;
    cubes_.erase(p);
    IVec3 np{p.x + d.x, p.y + d.y, p.z + d.z};
    placed:
    cubes_[np] = *c;
    newMembers.push_back(np);
  }
  it->second.swap(newMembers);
  return true;
}

} // namespace vxl
