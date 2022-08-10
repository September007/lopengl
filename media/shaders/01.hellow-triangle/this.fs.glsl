#version 330
in vec4 color;
out vec4 FragColor;
uniform vec4 uniform_color;
void main(){
    FragColor=uniform_color;
}