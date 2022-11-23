#version 120

attribute vec4 position;// vertex position
attribute vec2 TextureUV;// vertex texture coords


varying vec4 vs_output_position;// vertex position
varying vec2 vs_output_TextureUV;// vertex texture coords


void main(){
    vs_output_position =  position;
    vs_output_TextureUV = 
    //TextureUV;
    vec2(TextureUV.x,TextureUV.y)*1;
    gl_Position=position;
}
