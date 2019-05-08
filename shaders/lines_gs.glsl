#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform vec3 direction;

uniform mat4 projection;

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_Position = projection * gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position - vec4(direction, 1.0);
    gl_Position = projection * gl_Position;
    EmitVertex();

    EndPrimitive();
}