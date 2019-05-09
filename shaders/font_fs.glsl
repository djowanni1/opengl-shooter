#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D Texture;
uniform sampler2D health;
uniform sampler2D score;
uniform int number;

void main() {
    if (number >= 0){
        color = texture(Texture, TexCoord);
    } else if (number == -1){
        color = mix(texture(score, TexCoord), vec4(1.0, 0.0, 0.0, 1.0), 0.5);
    } else if (number == -2){
        color = mix(texture(health, TexCoord), vec4(0.0, 1.0, 0.0, 1.0), 0.5);
    } else {
        color = vec4(0.0);
    }

}