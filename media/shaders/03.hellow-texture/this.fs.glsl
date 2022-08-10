#version 330
in vec4 color;
in vec2 texCoord;
out vec4 FragColor;

uniform vec4 offset;
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

// use offset to change linear param
void main(){
    FragColor=
    mix(
        texture(ourTexture1,texCoord),
        texture(ourTexture2,texCoord),
    (offset.x+offset.y)+0.5);
}