#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/** @file camera.hpp
 *  @brief Orbit/FPS hybrid camera. Computes view/projection and pixel-true default distance.
 */

namespace vxl {

class Camera {
public:
  Camera();

  // View/projection
  void set_viewport(int w, int h);
  glm::mat4 view() const;
  glm::mat4 proj() const;

  // Orbit parameters
  void orbit(float dx, float dy); // radians based on mouse deltas scaled
  void pan(float dx, float dy);   // in world units along camera right/up
  void zoom(float amount);        // dolly in/out
  void set_target(const glm::vec3& t) { target_ = t; }
  glm::vec3 target() const { return target_; }
  float distance() const { return distance_; }
  void set_distance(float d) { distance_ = std::max(0.1f, d); }

  // Compute distance so that a unit cube edge appears as desired pixels
  void set_distance_for_pixel_edge(int pixels);

  float fov_y = glm::radians(50.0f);
  float near_clip = 0.05f;
  float far_clip = 2000.0f;

  // FPS move
  void move_local(float forward, float right, float up);

  // Facing
  glm::vec3 eye() const;
  glm::vec3 forward() const;

private:
  int width_ = 1280, height_ = 720;
  glm::vec3 target_{0,0,0};
  float azimuth_ = glm::radians(45.0f);
  float elevation_ = glm::radians(30.0f);
  float distance_ = 8.0f;
};

} // namespace vxl
