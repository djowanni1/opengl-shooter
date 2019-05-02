#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D Texture;
uniform mat3 animation;

void main() {
    color = texture(Texture, (animation * vec3(TexCoord.x, TexCoord.y, 1.0)).xy);
}