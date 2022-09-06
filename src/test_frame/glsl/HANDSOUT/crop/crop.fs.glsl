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


varying vec4 vs_output_position;// vertex position
varying vec2 vs_output_textureUV;// vertex texture coords
varying vec2 texRect;
varying vec2 texShowingRange;

void main(){
    
    //gl_FragColor=PS_2D(vs_output_textureUV);
    
    // vec2 relativeTexCoord= abs(vs_output_textureUV-vec2(0.5,0.5));
    // vec2 relativeTexCoordLimitation=texShowingRange*0.5;
    // if(relativeTexCoord.x>relativeTexCoordLimitation.x
    // || relativeTexCoord.y>relativeTexCoordLimitation.y)
    // discard;
    gl_FragColor = TEXTURE2D(overlay,vs_output_textureUV);
    // gl_FragColor=TEXTURE2D(overlay,vec2(
    //     vs_output_textureUV.x+roi_x,
    //     vs_output_textureUV.y+roi_y));
    //gl_FragColor = vec4(texShowingRange,0,1);
    // vec2 uv=vs_output_textureUV;
    //  gl_FragColor=((step(0.0, uv.x) - step(1.0,uv.x)) * (step(0.0, uv.y) - step(1.0, uv.y)))<=0?
    //  vec4(0,0,0,1):vec4(1,1,1,1);
}