 #version 120

 attribute vec4 position;
 attribute vec2 textureUV;

 varying vec4 vs_output_position;
 varying vec2 vs_output_textureUV;

 void main(){
    gl_Position=position;
    vs_output_position=position;
    vs_output_textureUV=textureUV;
 }