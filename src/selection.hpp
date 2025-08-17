#pragma once
#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>
#include "util.hpp"
#include "universe.hpp"

/** @file selection.hpp
 *  @brief Selection manager with ray casting and basic manipulation.
 */

namespace vxl {

class Selection {
public:
  void clear();
  void add(const IVec3& p);
  void toggle(const IVec3& p);
  bool contains(const IVec3& p) const;
  std::vector<IVec3> items() const;

  /// Ray-cast to nearest cube center-aligned AABB (size 1), returns coordinate if hit.
  static std::optional<IVec3> pick_cube(const Universe& U,
                                        const glm::vec3& rayOrigin,
                                        const glm::vec3& rayDir);

  /// Move all selected cubes by integer delta (preserves materials).
  void move(Universe& U, const IVec3& d);

  /// Rotate selection around its centroid by degrees on axis (x/y/z).
  void rotate(Universe& U, char axis, float degrees);

private:
  struct IVec3Hash { std::size_t operator()(const IVec3& k) const noexcept { return IVec3Hasher{}(k); } };
  std::unordered_set<IVec3, IVec3Hash> set_;
};

} // namespace vxl
