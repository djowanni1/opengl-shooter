#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform sampler2D Texture;
//uniform mat3 animation;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);

    TexCoord = texCoord;
//    if (texture(Texture, (animation * vec3(TexCoord.x, TexCoord.y, 1.0)).xy).w == 1.0){
//        gl_Position.z = gl_Position.w;
//    }
}
