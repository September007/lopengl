#version 120

#define SWITCH(var)int v_var = var;{do{
   #define CASE(val)}if(v_var == val){
   #define DEFAULT()};
#define ENDSWITCH()}while(false);

attribute vec4 position;
attribute vec2 textureUV;

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
uniform int rotateType;//0:clockwise 90;1:clockwise 180;2:clockwise 270;3:flip X; 4:flip Y

uniform int overlay_Width;
uniform int overlay_Height;
// if rotate 180k+90, the output vertex coord should be shrink into rectangle
void main(){
   float k = float(overlay_Height)/overlay_Width;
   vec2 rotateScale = vec2(1,1);
   if(rotateType==0 || rotateType==2)
   if( k<1. ){
      rotateScale.x = k*k;
   }else{
      rotateScale.y = 1/k;
   }
   vec2 pos = vs_output_position.xy;
   gl_Position = 
   vs_output_position = vec4(position.xy*rotateScale,0,1);
   vs_output_textureUV = textureUV;
}

