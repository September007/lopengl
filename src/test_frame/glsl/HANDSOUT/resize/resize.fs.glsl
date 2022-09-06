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
uniform sampler2D overlay;        
uniform float    g_fTime;                   
uniform int overlay_Width;
uniform int overlay_Height;
uniform int dst_Width;
uniform int dst_Height;
uniform int resize_type;
//resize_type = 0;//IRM_FULLSCREEN
//resize_type = 1;//IRM_ORIGINAL_SCALE
//resize_type = 2;//IRM_16_9
//resize_type = 3;//IRM_4_3

#define WS_PI 3.14159265358979323846



vec4 PS_2D(vec2 TextureUV){
    float f_srcWidth = float(overlay_Width);
    float f_srcHeight = float(overlay_Height);
    float f_dstWidth = float(dst_Width);
    float f_dstHeight = float(dst_Height);
   
    vec2 tc = TextureUV;

    // float scalFactorX = f_dstWidth / f_srcWidth;
	// float scalFactorY = f_dstHeight / f_srcHeight;
    float resizeCoord_x = tc.x;
	float resizeCoord_y = tc.y;
    float matt = 1.0;
    if(resize_type == 1){
        float src_ratio = f_srcWidth / f_srcHeight;
        float dst_ratio = f_dstWidth / f_dstHeight;
        float tmpDstH = ceil(f_dstWidth * f_srcHeight / f_srcWidth);
        if(f_dstHeight >= tmpDstH) {
            float dstH = tmpDstH / f_dstHeight ;
            float roiY0 = (1.0 - dstH) * 0.5;
            float roiY1 = roiY0 + dstH;
            matt = step(roiY0 ,tc.y) * step(tc.y , roiY1);
            resizeCoord_y = (tc.y - roiY0 ) / dstH;
        }else{
            float tmpDstW = ceil(f_dstHeight * src_ratio);
            float dstW =  tmpDstW / f_dstWidth;
            float roiX0 = (1.0 - dstW) * 0.5;
            float roiX1 = roiX0 + dstW;
            matt = step(roiX0 ,tc.x) * step(tc.x , roiX1);
            resizeCoord_x = (tc.x - roiX0) / dstW;
        }
    }
    vec2 uv = vec2(resizeCoord_x,resizeCoord_y);
	vec4 ovlCol = TEXTURE2D(overlay,uv) * matt;
	
	return  ovlCol;
}

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
void main(){
	gl_FragColor = PS_2D(vs_output_textureUV);	
}
