//#version 330 core
//out vec4 FragColor;
//
//in vec3 TexCoords;
//
//uniform samplerCube skybox;
//uniform sampler2D bg;
//void main() {
//    FragColor = texture(bg, TexCoords);
//    //FragColor = texture(skybox, TexCoords);
//}

#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D bg;

void main() {
    //color = vec4(1.0);
    color = texture(bg, TexCoord);
}