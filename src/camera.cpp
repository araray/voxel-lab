#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace vxl {

Camera::Camera() {}

void Camera::set_viewport(int w, int h) { width_ = std::max(1,w); height_ = std::max(1,h); }

glm::mat4 Camera::view() const {
  glm::vec3 dir{
    std::cos(elevation_) * std::cos(azimuth_),
    std::sin(elevation_),
    std::cos(elevation_) * std::sin(azimuth_)
  };
  glm::vec3 eyePos = target_ - dir * distance_;
  return glm::lookAt(eyePos, target_, glm::vec3(0,1,0));
}

glm::mat4 Camera::proj() const {
  float aspect = float(width_) / float(height_);
  return glm::perspective(fov_y, aspect, near_clip, far_clip);
}

void Camera::orbit(float dx, float dy) {
  azimuth_ += dx;
  elevation_ = std::clamp(elevation_ + dy, glm::radians(-89.0f), glm::radians(89.0f));
}

void Camera::pan(float dx, float dy) {
  // Move target along camera right/up
  glm::mat4 V = view();
  glm::vec3 right{V[0][0], V[1][0], V[2][0]};
  glm::vec3 up{V[0][1], V[1][1], V[2][1]};
  target_ += right * dx + up * dy;
}

void Camera::zoom(float amount) {
  distance_ = std::max(0.1f, distance_ * std::exp(-amount));
}

glm::vec3 Camera::eye() const {
  glm::mat4 V = view();
  glm::mat4 Vinv = glm::inverse(V);
  return glm::vec3(Vinv[3]);
}

glm::vec3 Camera::forward() const {
  glm::mat4 V = view();
  return -glm::vec3(V[0][2], V[1][2], V[2][2]);
}

void Camera::move_local(float forwardAmt, float rightAmt, float upAmt) {
  glm::mat4 V = view();
  glm::vec3 fwd = -glm::vec3(V[0][2], V[1][2], V[2][2]);
  glm::vec3 right{V[0][0], V[1][0], V[2][0]};
  glm::vec3 up{V[0][1], V[1][1], V[2][1]};
  target_ += fwd * forwardAmt + right * rightAmt + up * upAmt;
}

void Camera::set_distance_for_pixel_edge(int pixels) {
  // For a unit cube (size=1), projected edge length in pixels for height H and FOV_y is:
  // s_pixels = (H/2) * size / (tan(FOV_y/2) * distance)
  // Solve for distance.
  float H = float(height_);
  float size = 1.0f; // world units (edge)
  float s = std::max(1, pixels);
  float d = (H * size) / (2.0f * std::tan(fov_y * 0.5f) * s);
  set_distance(d);
}

} // namespace vxl
