#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

uniform mat4 projection;

out float alpha;

void main() {
    float step = 0.01;
    float alpha_big = 0.3;
    float alpha_small = 0.1;
    gl_Position = gl_in[0].gl_Position;
    gl_Position = projection * gl_Position;
    alpha = alpha_big;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.0, step, 0.0, 1.0);
    gl_Position = projection * gl_Position;
    alpha = alpha_small;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(step, 0.0, 0.0, 1.0);
    gl_Position = projection * gl_Position;
    alpha = alpha_big;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.0, -step, 0.0, 1.0);
    gl_Position = projection * gl_Position;
    alpha = alpha_small;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-step, 0.0, 0.0, 1.0);
    gl_Position = projection * gl_Position;
    alpha = alpha_big;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.0, step, 0.0, 1.0);
    gl_Position = projection * gl_Position;
    alpha = alpha_small;
    EmitVertex();

    EndPrimitive();
}