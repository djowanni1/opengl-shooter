#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int number;

void main() {
    if (number >= 0){
        gl_Position = projection * view * model * (vec4(position, 50.0f)) / 50;
        TexCoord = vec2((texCoord.x + number) / 10, texCoord.y);
    } else if (number == -1){
        vec3 curpos = position;
        curpos.y /= 6.0;
        curpos /= 8.0;
        gl_Position = projection * view * model * (vec4(curpos, 1.0f));
        TexCoord = texCoord;
    } else if (number == -2){
        vec3 curpos = position;
        curpos.y /= 7.0;
        curpos /= 7.0;
        gl_Position = projection * view * model * (vec4(curpos, 1.0f));
        TexCoord = texCoord;
    } else {
        gl_Position = vec4(position, 1.0);
        TexCoord = texCoord;
    }

}
