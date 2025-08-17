#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include "util.hpp"

/** @file universe.hpp
 *  @brief Sparse, unbounded voxel universe. All cubes have same base geometry.
 */

namespace vxl {

/** @brief Material coloration model for a cube. */
struct Material {
  enum class Kind { Solid, Gradient } kind = Kind::Solid;
  glm::vec4 colorA{0.8f,0.8f,0.8f,1.0f};
  glm::vec4 colorB{0.2f,0.2f,0.2f,1.0f};
  glm::vec3 gradDir{0,1,0}; ///< unit direction in local cube space
};

/** @brief Optional hook to allow embedding custom renderables per-cube. */
struct IEmbeddedRenderable {
  virtual ~IEmbeddedRenderable() = default;
  /// Called with model matrix that maps cube-local coordinates to world.
  virtual void draw(const glm::mat4& cubeModel) = 0;
};

/** @brief One cube at an integer coordinate. */
struct Cube {
  Material mat{};
  glm::quat rotation{1,0,0,0};  ///< local rotation
  IEmbeddedRenderable* embedded{nullptr}; ///< user-owned object (not managed)
};

/** @brief Universe parameters and sparse content. */
class Universe {
public:
  /// Construct with desired on-screen pixels for cube edge at default zoom.
  explicit Universe(int baseEdgePixels = 64);

  int base_edge_pixels() const noexcept { return baseEdgePixels_; }
  void set_base_edge_pixels(int px) noexcept { baseEdgePixels_ = std::max(1, px); }

  /// Place or replace a cube.
  void place(int x,int y,int z, const Cube& c = Cube{});
  /// Remove cube if present. Returns true if erased.
  bool erase(int x,int y,int z);
  /// Get cube if present.
  std::optional<Cube> get(int x,int y,int z) const;

  /// Bulk access
  const std::unordered_map<IVec3, Cube, IVec3Hasher>& all() const { return cubes_; }

  // Groups
  void group_create(const std::string& name, const std::vector<IVec3>& members);
  bool group_exists(const std::string& name) const;
  std::vector<IVec3> group_members(const std::string& name) const;
  bool group_erase(const std::string& name);
  bool group_move(const std::string& name, const IVec3& d);

private:
  int baseEdgePixels_;
  std::unordered_map<IVec3, Cube, IVec3Hasher> cubes_;
  std::unordered_map<std::string, std::vector<IVec3>> groups_;
};

} // namespace vxl
