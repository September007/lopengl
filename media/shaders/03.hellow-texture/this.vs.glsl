#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
layout(location=2)in vec2 aTexCoord;
uniform vec4 offset;
out vec4 color;
out vec2 texCoord;
void main(){
   // gl_Position=vec4(aPos,1)+vec4(offset[0],offset[1],offset[2],0);
    gl_Position=vec4(aPos,1)+offset;
    texCoord=aTexCoord;
    color=vec4(aCol,1.f);
}