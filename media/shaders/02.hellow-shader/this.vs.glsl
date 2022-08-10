#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
out vec4 color;
void main(){
    gl_Position=vec4(aPos,1);
    color=vec4(aCol,1.f);
}