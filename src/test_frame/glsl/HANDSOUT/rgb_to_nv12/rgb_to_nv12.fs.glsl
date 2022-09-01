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
//clang-format on





//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
uniform sampler2D overlay;            //

uniform int mode;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int dst_Width;
uniform int dst_Height;



#define EQN_EPS 1e-9f

bool isZero(float x) {
	return (x > -EQN_EPS && x < EQN_EPS);
}

vec4 PS_2D(vec2 TextureUV){
    float ow = float(overlay_Width); 
    float oh = float(overlay_Height);
    float bw = float(dst_Width);
    float bh = float(dst_Height);
    vec2 tc = TextureUV;
    
    vec4 img = TEXTURE2D(overlay,tc) * 255.0f;
    vec4 yuv = vec4(0.0,0.0,0.0,0.0); 
    img.xyz = img.zyx;
    
    if(mode == 0){
        //bt709
        yuv.x = 0.1825 * img.z + 0.6142 * img.y + 0.0620 * img.x + 16.0;        

    }else{
        yuv.y = -0.1006f*img.z - 0.3385f*img.y + 0.4392f*img.x + 128.0f;
        yuv.z = 0.4392f*img.z - 0.3989f*img.y - 0.0402f*img.x + 128.0f;
    }
    yuv  = yuv / 255.0;
    return vec4(
        yuv.x,
        yuv.y,
        yuv.z,
        yuv.a
    );
    return  yuv;

}

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;

void main(){
	gl_FragColor = PS_2D(vs_output_textureUV);	
}
