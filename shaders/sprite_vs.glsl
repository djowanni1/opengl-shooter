#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

//uniform int sprite_step;
//out int Step;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

    gl_Position = projection * view * model * vec4(position, 1.0f);

    TexCoord = texCoord;

    //Step = sprite_step;
}
