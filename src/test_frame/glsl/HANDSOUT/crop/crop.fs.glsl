#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

#define Texture2D sampler2D

uniform Texture2D overlay;
uniform float roi_x;
uniform float roi_y;
uniform int overlay_Width;
uniform int overlay_Height;
// ot use yet
uniform int dst_Width;
uniform int dst_Height;

//(float\d+)( +)(\w+)( +):( +)(\w+)
//$1$2$3
struct VS_INPUT
{
    vec4 Position;// vertex position
    vec2 TextureUV;// vertex texture coords
};

struct VS_OUTPUT
{
    vec4 Position;// vertex position
    vec2 TextureUV;// vertex texture coords
};

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    vec4 RGBColor;// Pixel color
};

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
vec2 rotateFunc(vec2 uv,vec2 center,float theta)
{
    vec2 temp;
    temp.x=dot(vec2(cos(theta),-sin(theta)),uv-center);
    temp.y=dot(vec2(sin(theta),cos(theta)),uv-center);
    return(temp+center);
}

vec4 PS_2D(vec2 tc)
{
    PS_OUTPUT Output;
    float widthScale=float(dst_Width)/overlay_Width;
    float heightScale=float(dst_Height)/overlay_Height;
    vec2 scaledTC = vec2(tc.x*widthScale, tc.y*heightScale);
    // what if dst.[x|y] > overlay.[x|y] ?
    vec2 uv=vec2(tc.x*widthScale+roi_x,tc.y*heightScale+roi_y);
    
    return TEXTURE2D(overlay,uv);
    
    //float grid = (step(0.0, uv.x) - step(1.0,uv.x)) * (step(0.0, uv.y) - step(1.0, uv.y));
    //vec4 ovlCol = TEXTURE2D(overlay,uv)*(1-grid);
    //return ovlCol;
}

varying vec4 vs_output_position;// vertex position
varying vec2 vs_output_textureUV;// vertex texture coords

void main(){
    
    gl_FragColor=PS_2D(vs_output_textureUV);
    
    //gl_FragColor=TEXTURE2D(overlay,vs_output_textureUV);
    // vec2 uv=vs_output_textureUV;
    //  gl_FragColor=((step(0.0, uv.x) - step(1.0,uv.x)) * (step(0.0, uv.y) - step(1.0, uv.y)))<=0?
    //  vec4(0,0,0,1):vec4(1,1,1,1);
}