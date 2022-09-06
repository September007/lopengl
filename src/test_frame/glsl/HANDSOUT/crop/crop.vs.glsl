#version 120

attribute vec4 position;
attribute vec2 textureUV;

uniform float roi_x;
uniform float roi_y;
uniform int overlay_Width;
uniform int overlay_Height;
// ot use yet
uniform int dst_Width;
uniform int dst_Height;

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
// uniform
varying vec2 texRect;
varying vec2 texShowingRange;
void main(){
   
   vec2 texLeftBottom=vec2(roi_x,roi_y);
   vec2 texRightTop=vec2(roi_x+float(dst_Width)/overlay_Width,roi_y+float(dst_Height)/overlay_Height);
   texRightTop.x=min(texRightTop.x,1.);
   texRightTop.y=min(texRightTop.y,1.);
   
   // 被采样的纹理范围
   texRect =vec2(texRightTop.x-texLeftBottom.x,texRightTop.y-texLeftBottom.y);
   float bigggerDim=max(texRect.x,texRect.y);
   float scaleRate = 1/bigggerDim;
   // 被渲染的 标准化设备坐标 范围的大小
   texShowingRange = texRect*scaleRate; 


   // 缩小渲染范围
   gl_Position=
   vs_output_position=vec4(position.x*texShowingRange.x,position.y*texShowingRange.y,0,1);
   // 重定位采样点
   vs_output_textureUV= vec2(roi_x,roi_y)+vec2(textureUV.x*texRect.x,textureUV.y*texRect.y);
}