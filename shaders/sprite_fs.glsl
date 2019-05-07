#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D Texture;
uniform sampler2D boom;
uniform bool is_alive;
uniform mat3 animation;

void main() {
    if (is_alive){
        color = texture(Texture, (animation * vec3(TexCoord.x, TexCoord.y, 1.0)).xy);
    } else {
        color = texture(boom, (animation * vec3(TexCoord.x, TexCoord.y, 1.0)).xy);
    }
}