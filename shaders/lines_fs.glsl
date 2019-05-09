#version 330 core
out vec4 color;
uniform vec3 incolor;

void main() {
    color = vec4(incolor, 1.0);
    //color = vec4(1.0);
}