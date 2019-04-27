#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoord;
out vec3 BgCoord;

uniform int tex_type;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    if (tex_type == 2){
        gl_Position = projection * view * vec4(position, 1.0f);
        //gl_Position.z = gl_Position.w;
       //view = mat4(mat3(view));
    } else {
        gl_Position = projection * view * model * vec4(position, 1.0f);
    }

    TexCoord = texCoord;
    BgCoord = position;
}
