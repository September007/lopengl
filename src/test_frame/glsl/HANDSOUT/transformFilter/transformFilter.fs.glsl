#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

#define Texture2D  sampler2D  
// clang-format off
#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
uniform Texture2D overlay;        
uniform float dScaleX;
uniform float dScaleY;
uniform float theta;
uniform float    g_fTime;                   
uniform int overlay_Width;
uniform int overlay_Height;
uniform int dst_Width;
uniform int dst_Height;

#define WS_PI 3.14159265358979323846


vec2 rotateFunc(vec2 uv, vec2 center, float theta)
{
	vec2 temp;
	temp.x = dot(vec2(cos(theta), -sin(theta)), uv - center);
	temp.y = dot(vec2(sin(theta), cos(theta)), uv - center);
	return (temp + center);
}

vec4 PS_2D(vec2 TextureUV){
    float ow = float(overlay_Width);
    float oh = float(overlay_Height);
   
    vec2 tc = TextureUV;
    float deg = theta * WS_PI / 180.0;
    float dCosA = cos(deg);
    float dSinA = sin(deg);
    float xCerten = ow * 0.5;  
    float yCerten = oh * 0.5;
    vec2 center = vec2(0.5,0.5);
    vec2 uv = rotateFunc(tc,center ,deg) ;
    float scalFactorX = float(dst_Width) / ow;
	float scalFactorY = float(dst_Height) / oh;
    uv.x = (uv.x - center.x) /  dScaleX  * scalFactorX + center.x;
    uv.y = (uv.y - center.y) /  dScaleY * scalFactorY + center.y;
    float grid = (step(0.0, uv.x) - step(1.0,uv.x)) * (step(0.0, uv.y) - step(1.0, uv.y));   
	vec4 ovlCol = TEXTURE2D(overlay,uv)*grid;
	
	return  ovlCol;
}


varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
void main(){
    gl_FragColor = PS_2D(vs_output_textureUV);
}