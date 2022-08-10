#version 330
in vec4 color;
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main(){
    FragColor=texture(ourTexture,texCoord)*color;
}