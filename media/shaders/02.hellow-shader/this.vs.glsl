#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;

out vec2 texCoord;
uniform vec4 offset;
out vec4 color;
void main(){
   // gl_Position=vec4(aPos,1)+vec4(offset[0],offset[1],offset[2],0);
    texCoord=gl_Position=vec4(aPos,1)+offset;
    color=vec4(aCol,1.f);
}