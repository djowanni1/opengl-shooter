#version 330 core
in vec2 TexCoord;
in vec3 BgCoord;

out vec4 color;

uniform int tex_type;

uniform sampler2D Texture1;
uniform sampler2D Texture2;
uniform samplerCube Texture3;

void main() {
    if (tex_type == 0) {
        color = texture(Texture1, TexCoord);
    } else if (tex_type == 1) {
        color = texture(Texture2, TexCoord);
    } else if (tex_type == 2) {
        color = texture(Texture3, vec3(BgCoord.x, BgCoord.y, BgCoord.z));
    }
    //sampler2D *textures[] = {&(Texture1), &Texture2, &Texture3};
    //color = texture(textures[tex_type], TexCoord);

    //color = mix(texture(Texture1, TexCoord), texture(Texture2, TexCoord), 0.5);
    //color = texture(Texture1, TexCoord);
    //color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}