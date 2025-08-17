#include "renderer.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

#ifndef DEFAULT_SHADER_DIR
#define DEFAULT_SHADER_DIR "."
#endif

namespace vxl {

static std::string read_text_file(const std::string& path) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("Cannot read file: " + path);
  std::stringstream ss; ss << f.rdbuf();
  return ss.str();
}

static unsigned int compile_shader(GLenum type, const std::string& src) {
  GLuint s = glCreateShader(type);
  const char* c = src.c_str();
  glShaderSource(s, 1, &c, nullptr);
  glCompileShader(s);
  GLint ok=0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint len=0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0'); glGetShaderInfoLog(s, len, nullptr, log.data());
    throw std::runtime_error("Shader compile failed: " + log);
  }
  return s;
}

Renderer::Renderer() {}
Renderer::~Renderer() {
  if (vboInst_) glDeleteBuffers(1,&vboInst_);
  if (vboVerts_) glDeleteBuffers(1,&vboVerts_);
  if (ebo_) glDeleteBuffers(1,&ebo_);
  if (vao_) glDeleteVertexArrays(1,&vao_);
  if (prog_) glDeleteProgram(prog_);
}

void Renderer::init_gl() {
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) throw std::runtime_error("GLEW init failed");
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  build_program();
  build_cube_mesh();
}

void Renderer::resize(int w, int h) { viewportW_ = w; viewportH_ = h; }

void Renderer::build_program() {
  std::string vsPath = std::string(DEFAULT_SHADER_DIR) + "/cube.vert";
  std::string fsPath = std::string(DEFAULT_SHADER_DIR) + "/cube.frag";
  auto vs = compile_shader(GL_VERTEX_SHADER, read_text_file(vsPath));
  auto fs = compile_shader(GL_FRAGMENT_SHADER, read_text_file(fsPath));
  prog_ = glCreateProgram();
  glAttachShader(prog_, vs); glAttachShader(prog_, fs);
  glLinkProgram(prog_);
  GLint ok=0; glGetProgramiv(prog_, GL_LINK_STATUS, &ok);
  if (!ok) {
    GLint len=0; glGetProgramiv(prog_, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0'); glGetProgramInfoLog(prog_, len, nullptr, log.data());
    throw std::runtime_error("Program link failed: " + log);
  }
  glDeleteShader(vs); glDeleteShader(fs);
}

void Renderer::build_cube_mesh() {
  // cube vertices (pos, normal). 36 vertices indexed (12 triangles); also allow instancing
  struct V { float x,y,z; float nx,ny,nz; };
  std::vector<V> verts;
  std::vector<unsigned int> idx;

  auto add_face = [&](glm::vec3 n, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d){
    unsigned int base = (unsigned int)verts.size();
    auto push = [&](glm::vec3 p){ verts.push_back({p.x,p.y,p.z, n.x,n.y,n.z}); };
    push(a); push(b); push(c); push(d);
    idx.push_back(base+0); idx.push_back(base+1); idx.push_back(base+2);
    idx.push_back(base+0); idx.push_back(base+2); idx.push_back(base+3);
  };

  float s=0.5f;
  add_face({ 0, 0, 1}, {-s,-s, s}, { s,-s, s}, { s, s, s}, {-s, s, s});
  add_face({ 0, 0,-1}, {-s,-s,-s}, {-s, s,-s}, { s, s,-s}, { s,-s,-s});
  add_face({ 0, 1, 0}, {-s, s, s}, { s, s, s}, { s, s,-s}, {-s, s,-s});
  add_face({ 0,-1, 0}, {-s,-s, s}, {-s,-s,-s}, { s,-s,-s}, { s,-s, s});
  add_face({ 1, 0, 0}, { s,-s, s}, { s,-s,-s}, { s, s,-s}, { s, s, s});
  add_face({-1, 0, 0}, {-s,-s, s}, {-s, s, s}, {-s, s,-s}, {-s,-s,-s});

  glGenVertexArrays(1,&vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1,&vboVerts_);
  glBindBuffer(GL_ARRAY_BUFFER, vboVerts_);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(V), verts.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)0);
  glEnableVertexAttribArray(1); // normal
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)(3*sizeof(float)));

  glGenBuffers(1,&ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

  // Instance buffer: model matrix (4 vec4), colorA (vec4), colorB(vec4), kind(int), gradDir(vec3)
  glGenBuffers(1,&vboInst_);
  glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
  size_t stride = sizeof(float)* (16 + 4 + 4 + 1 + 3);
  std::size_t offset = 0;
  for (int i=0;i<4;i++) {
    glEnableVertexAttribArray(2+i);
    glVertexAttribPointer(2+i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
    glVertexAttribDivisor(2+i, 1);
    offset += sizeof(float)*4;
  }
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
  glVertexAttribDivisor(6,1);
  offset += sizeof(float)*4;
  glEnableVertexAttribArray(7);
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
  glVertexAttribDivisor(7,1);
  offset += sizeof(float)*4;
  glEnableVertexAttribArray(8);
  glVertexAttribIPointer(8, 1, GL_INT, stride, (void*)(offset));
  glVertexAttribDivisor(8,1);
  offset += sizeof(int)*1;
  glEnableVertexAttribArray(9);
  glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
  glVertexAttribDivisor(9,1);

  glBindVertexArray(0);
}

void Renderer::draw_grid(const glm::mat4& VP) const {
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_LINES);
  const int R = 50;
  for (int i=-R;i<=R;i++) {
    glVertex3f((float)i, 0.0f, (float)-R);
    glVertex3f((float)i, 0.0f, (float) R);
    glVertex3f((float)-R, 0.0f, (float)i);
    glVertex3f((float) R, 0.0f, (float)i);
  }
  glEnd();
  glEnable(GL_DEPTH_TEST);
}

void Renderer::render(const Universe& U, const Selection& Sel, const glm::mat4& V, const glm::mat4& P,
                      bool drawGrid) {
  glViewport(0,0,viewportW_, viewportH_);
  glClearColor(0.08f,0.09f,0.1f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (drawGrid) {
    // Quick GL fixed pipeline line grid (compat) – fine for overlay
    // (Modern approach would use a dedicated shader; kept simple here.)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    draw_grid(P*V);
  }

  // Prepare instance buffer
  struct Instance {
    float model[16];
    float colorA[4];
    float colorB[4];
    int kind;
    float gradDir[3];
  };
  std::vector<Instance> inst;
  inst.reserve(U.all().size());

  for (auto&& [p, c] : U.all()) {
    glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(p.x, p.y, p.z)) * glm::mat4_cast(c.rotation);
    Instance I{};
    std::memcpy(I.model, glm::value_ptr(M), sizeof(float)*16);
    std::memcpy(I.colorA, glm::value_ptr(c.mat.colorA), sizeof(float)*4);
    std::memcpy(I.colorB, glm::value_ptr(c.mat.colorB), sizeof(float)*4);
    I.kind = (c.mat.kind == Material::Kind::Solid) ? 0 : 1;
    I.gradDir[0]=c.mat.gradDir.x; I.gradDir[1]=c.mat.gradDir.y; I.gradDir[2]=c.mat.gradDir.z;
    inst.push_back(I);
  }

  glUseProgram(prog_);
  GLint locV = glGetUniformLocation(prog_, "uView");
  GLint locP = glGetUniformLocation(prog_, "uProj");
  glm::mat4 Vcopy = V, Pcopy = P;
  glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(Vcopy));
  glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(Pcopy));

  if (wireframe_) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
  glBufferData(GL_ARRAY_BUFFER, inst.size()*sizeof(Instance), inst.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

  // There are 36 indices total
  GLsizei indexCount = 36;
  glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, (GLsizei)inst.size());

  if (wireframe_) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace vxl
