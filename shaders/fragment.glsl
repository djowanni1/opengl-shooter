#version 330 core
in vec2 TexCoord;
in vec3 BgCoord;

out vec4 color;

uniform sampler2D Texture1;
//uniform sampler2D Texture2;

void main() {
    color = texture(Texture1, TexCoord);
    //sampler2D *textures[] = {&(Texture1), &Texture2, &Texture3};
    //color = texture(textures[tex_type], TexCoord);

    //color = mix(texture(Texture1, TexCoord), texture(Texture2, TexCoord), 0.5);
    //color = texture(Texture1, TexCoord);
    //color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}