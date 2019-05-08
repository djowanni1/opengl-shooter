#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform vec3 direction;

void main() {
    gl_Position = gl_in[0].gl_Position + vec4(direction, 1.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position - vec4(direction, 1.0);
    EmitVertex();

    EndPrimitive();
}