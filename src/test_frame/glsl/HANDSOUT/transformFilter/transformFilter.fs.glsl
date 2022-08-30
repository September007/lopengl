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


struct VS_INPUT
{
    vec4 Position; // vertex position 
    vec2 TextureUV;   // vertex texture coords 
};

struct VS_OUTPUT
{
    vec4 Position; // vertex position 
    vec2 TextureUV;   // vertex texture coords 
};
VS_OUTPUT vs_output;
 varying vec4 vs_output_position;
 varying vec2 vs_output_textureUV;

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    vec4 RGBColor;  // Pixel color
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
vec2 rotateFunc(vec2 uv, vec2 center, float theta)
{
	vec2 temp;
	temp.x = dot(vec2(cos(theta), -sin(theta)), uv - center);
	temp.y = dot(vec2(sin(theta), cos(theta)), uv - center);
	return (temp + center);
}

PS_OUTPUT PS_2D( VS_OUTPUT In)
{ 
    PS_OUTPUT Output;
    float ow = float(overlay_Width);
    float oh = float(overlay_Height);
   
    vec2 tc = In.TextureUV;
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
	
	Output.RGBColor = ovlCol;
    return Output;
}

// 
VS_OUTPUT VS_2D(VS_INPUT vin)
{
	VS_OUTPUT vOut;
	vOut.Position = vin.Position;
	vOut.TextureUV = vin.TextureUV;
	return vOut;
}



void main(){
    vs_output.Position=vs_output_position;
    vs_output.TextureUV=vs_output_textureUV;
    
    PS_OUTPUT ps_output=PS_2D(vs_output);

    gl_FragColor=ps_output.RGBColor;
}