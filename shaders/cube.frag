#version 330 core
in VS_OUT {
    vec3 normal;
    vec3 worldPos;
    vec4 colorA;
    vec4 colorB;
    flat int kind;
    vec3 gradDir;
} fs_in;

out vec4 FragColor;

void main() {
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(vec3(0.6, 1.0, 0.3));
    float ndl = max(dot(N, L), 0.0);
    vec3 base;
    if (fs_in.kind == 0) {
        base = fs_in.colorA.rgb;
    } else {
        // Gradient along dir in local space approximated by world normal-space dot
        float t = 0.5 + 0.5 * dot(N, normalize(fs_in.gradDir));
        base = mix(fs_in.colorA.rgb, fs_in.colorB.rgb, t);
    }
    vec3 color = (0.15 + 0.85 * ndl) * base;
    FragColor = vec4(color, fs_in.colorA.a);
}
