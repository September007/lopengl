#version 120

varying vec2 TexCoord;

uniform sampler2D aTexture;

void main(){
    gl_FragColor=texture2D(aTexture,TexCoord);
}

