#pragma once
#include <glm/glm.hpp>
#include "universe.hpp"
#include "selection.hpp"

/** @file renderer.hpp
 *  @brief OpenGL renderer: cubes (instanced), grid, selection outlines, wireframe toggle.
 */

namespace vxl {

class Renderer {
public:
  Renderer();
  ~Renderer();

  void init_gl();                    ///< Create GL resources; call after GL context ready
  void resize(int w, int h);
  void set_wireframe(bool on) { wireframe_ = on; }

  /// Draw a frame.
  void render(const Universe& U, const Selection& Sel, const glm::mat4& V, const glm::mat4& P,
              bool drawGrid);

private:
  // GL resources
  unsigned int prog_ = 0;
  unsigned int vao_ = 0, vboVerts_ = 0, vboInst_ = 0, ebo_ = 0;
  int viewportW_ = 1, viewportH_ = 1;

  bool wireframe_ = false;

  void build_program();
  void build_cube_mesh();
  void draw_grid(const glm::mat4& VP) const;
};

} // namespace vxl
