#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;

// Instance attributes
layout (location=2) in vec4 iM0;
layout (location=3) in vec4 iM1;
layout (location=4) in vec4 iM2;
layout (location=5) in vec4 iM3;
layout (location=6) in vec4 iColorA;
layout (location=7) in vec4 iColorB;
layout (location=8) in int  iKind;
layout (location=9) in vec3 iGradDir;

uniform mat4 uView;
uniform mat4 uProj;

out VS_OUT {
  vec3 normal;
  vec3 worldPos;
  vec4 colorA;
  vec4 colorB;
  int  kind;
  vec3 gradDir;
} vs_out;

void main() {
  mat4 M = mat4(iM0, iM1, iM2, iM3);
  vec4 wp = M * vec4(aPos, 1.0);
  gl_Position = uProj * uView * wp;
  vs_out.worldPos = wp.xyz;
  vs_out.normal = mat3(M) * aNormal;
  vs_out.colorA = iColorA;
  vs_out.colorB = iColorB;
  vs_out.kind = iKind;
  vs_out.gradDir = normalize(iGradDir);
}
