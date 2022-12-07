#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif
//#define Texture2D sampler2D

uniform sampler2D tex_Y;// Color texture for mesh
uniform sampler2D tex_UV;// Color texture for mesh
uniform int mode;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int back_Width;
uniform int back_Height;
// clang-format off
#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);
// clang-format on

#define EQN_EPS 1e-9f

vec4 funcNv12ToRGBABT709CSC(vec3 yuv)
{
    vec3 rgb;
    rgb.x=1.1644*(yuv.x-16.)+1.7927*(yuv.z-128.);
    rgb.y=1.1644*(yuv.x-16.)-.5329*(yuv.z-128.)-.2132*(yuv.y-128.);
    rgb.z=1.1644*(yuv.x-16.)+2.1123*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT. 709, color Range: Full.  (HDTV)
vec4 funcNv12ToRGBABT709HDTV(vec3 yuv)
{
    vec3 rgb;
    rgb.x=yuv.x+1.5748*(yuv.z-128.);
    rgb.y=yuv.x-.4681*(yuv.z-128.)-.1873*(yuv.y-128.);
    rgb.z=yuv.x+1.8556*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT.601 PAL/ BT.601 NTSC /SMPTE 240M, color Range:Limited (CSC)
vec4 funcNv12ToRGBABT601CSC(vec3 yuv)
{
    vec3 rgb;
    rgb.x=1.1644*(yuv.x-16.)+1.5960*(yuv.z-128.);
    rgb.y=1.1644*(yuv.x-16.)-.8129*(yuv.z-128.)-.3917*(yuv.y-128.);
    rgb.z=1.1644*(yuv.x-16.)+2.0172*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT.601 PAL/ BT.601 NTSC /SMPTE 240M,, color Range:Full
vec4 funcNv12ToRGBABT601HDTV(vec3 yuv)
{
    vec3 rgb;
    rgb.x=yuv.x+1.4746*(yuv.z-128.);
    rgb.y=yuv.x-.5713*(yuv.z-128.)-.1645*(yuv.y-128.);
    rgb.z=yuv.x+1.8814*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

vec4 choosMode(vec3 yuv,int _mode)
{
    
    yuv=yuv*255.;
    vec4 outColor;
    SWITCH(_mode)
    {
        CASE(0)//BT. 709, color Range:Limited.   (CSC) , default mode
        outColor=funcNv12ToRGBABT709CSC(yuv);
        //outColor=vec4(1,0,0,1);
        break;
        CASE(1)//BT. 709, color Range: Full.
        outColor=funcNv12ToRGBABT709HDTV(yuv);
        //outColor=vec4(0.0902, 0.6196, 0.4431, 1.0);
        break;
        CASE(2)//BT.601 PAL/ BT.601 NTSC /SMPTE 240M, color Range:Limited
        outColor=funcNv12ToRGBABT601CSC(yuv);
        break;
        CASE(3)//BT.601 PAL/ BT.601 NTSC /SMPTE 240M,, color Range:Full
        outColor=funcNv12ToRGBABT601HDTV(yuv);
        break;
        DEFAULT()
        outColor=funcNv12ToRGBABT601CSC(yuv);
        break;
        ENDSWITCH()
    }
    return outColor;
}
bool isZero(float x){
    return(x>-EQN_EPS&&x<EQN_EPS);
}

int mod(in int i,int m){
    return i-i/m*m;
}

// tc: coord of texture pixel
vec4 PS3_2D(vec2 tc)
{
    // int scale=2;
    // vec2 block=
    // vec2(14,9);
    // block=vec2(1,1);
    // ivec2 crd=ivec2(
    //    0,0
    //     );
    // tc=vec2((tc.x+crd.x)/block.x,(tc.y+crd.y)/block.y);
    SWITCH(0)
    {
        CASE(0)

        // vec2 foverlay = vec2(overlay_Width,overlay_Height);

        // ivec2 itc = ivec2(tc.x*overlay_Width,tc.y*overlay_Height);
        // ivec2 itc_y = itc;
        // ivec2 itc_u = ivec2(itc.x/2*2,itc.y/2);
        // ivec2 itc_v = ivec2(itc.x/2*2+1,itc.y/2);
        
        // vec2 tc_y = vec2(float(itc_y.x)/float(overlay_Width),itc_y.y/float(overlay_Height));//itc_y/foverlay;
        // vec2 tc_u = vec2((float(itc_u.x))/float(overlay_Width),itc_u.y/float(overlay_Height));//itc_u/foverlay;
        // vec2 tc_v = vec2((float(itc_v.x+0.1))/float(overlay_Width),itc_v.y/float(overlay_Height));//itc_y/foverlay;
        
        // vec4 Ycolor=TEXTURE2D(tex_Y,tc_y);
        // vec4 Ucolor=TEXTURE2D(tex_UV,tc_u);
        // vec4 Vcolor=TEXTURE2D(tex_UV,tc_v);

        // float ux_int_coord = (int(( tc.x *overlay_Width)))/2*2;
        // float vx_int_coord=ux_int_coord+1;

        // vec4 Ycolor=TEXTURE2D(tex_Y,tc);
        // vec4 Ucolor=TEXTURE2D(tex_UV,vec2(ux_int_coord/overlay_Width,tc.y));
        // vec4 Vcolor=TEXTURE2D(tex_UV,vec2(vx_int_coord/overlay_Width,tc.y));
        if(tc.x>1||tc.x<0) discard;
        
        int yy_int=int(tc.y*overlay_Height);
        int uy_int=yy_int/2;
        // int ux_int=int((float(mod(yy_int,2))+tc.x )*overlay_Width/2);
        // vec2 UC=vec2(float(ux_int)/overlay_Width,float(uy_int)/(overlay_Height/2));
        int ux_int=int((float(mod(yy_int,2))+tc.x )*overlay_Width/2);
        vec2 UC=vec2(float(ux_int)/overlay_Width,float(uy_int)/(overlay_Height/2));
        //return vec4(UC,0,1);
        //UC=vec2(0.1,0.4);
        vec4 Ycolor=TEXTURE2D(tex_Y,tc);
        vec4 Ucolor=TEXTURE2D(tex_UV,UC);
        vec4 Vcolor=TEXTURE2D(tex_UV,vec2(float(ux_int+1)/overlay_Width,float(uy_int)/(overlay_Height/2)));
        if(UC.x>1||UC.x<0)discard;
        if(UC.y>1||UC.y<0)discard; 

        vec3 yuv=vec3(
        Ycolor.x
        ,
        Ucolor.x
        ,
        Vcolor.x
        );  
        //yuv.yz=UC.xy;
        vec4 color=vec4(yuv,1);
        //color=vec4(UC,0,1);
        //=color=choosMode(yuv,mode);
        SWITCH( 4 ){
            CASE(0)
            return color;
            CASE(1)
            return TEXTURE2D(tex_UV,UC);
            CASE(2)
            return TEXTURE2D(tex_UV,tc);
            CASE(3)
            return vec4(UC,0,1);
            CASE(4)
            return vec4(Ucolor);
            CASE(5)
            return vec4(Vcolor);
            
            DEFAULT()
            ENDSWITCH()
        }
        CASE(1)
        discard;
        float bw=float(back_Width);
        float bh=float(back_Height);
        float mw=float(overlay_Width);
        float mh=float(overlay_Height)/1.5;
        
        float w=tc.x*mw;
        float h=tc.y*overlay_Height;
        if(h<mh)
        {
            int uvw=int(w)/2*2;
            float y_y=tc.y*2./3.;
            vec4 Ycolor=TEXTURE2D(tex_Y,tc);
            vec4 UVcolor=TEXTURE2D(tex_UV,tc);
            vec4 color=vec4(1.);
            vec3 yuv=vec3(Ycolor.x,UVcolor.x,UVcolor.y);
            
            color=choosMode(yuv,mode);
            return color;
        }
        break;
        DEFAULT()
        ENDSWITCH()
    };
    
    //method2
    // {
        
    // }
    
}
varying vec4 vs_output_position;// vertex position
varying vec2 vs_output_TextureUV;// vertex texture coords

void main(){

    vec4 xy=vs_output_position; 
    vec2
    tc=vs_output_TextureUV;
    if(tc.x<0||tc.x>1
        )
        discard;


    gl_FragColor=PS3_2D(tc);
}
